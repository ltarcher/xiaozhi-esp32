#include "universal_http_client.h"
#include <esp_log.h>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

static const char* TAG = "UniversalHttpClient";

UniversalHttpClient::UniversalHttpClient(NetworkInterface* network_interface) 
    : network_interface_(network_interface) {
    ESP_LOGI(TAG, "Creating UniversalHttpClient with network interface: %p", network_interface_);
}

UniversalHttpClient::~UniversalHttpClient() {
    ESP_LOGI(TAG, "Destroying UniversalHttpClient");
    Close();
}

void UniversalHttpClient::SetProxy(const std::string& host, int port) {
    proxy_config_.host = host;
    proxy_config_.port = port;
    ESP_LOGI(TAG, "Proxy set: %s:%d", host.c_str(), port);
}

void UniversalHttpClient::SetProxy(const ProxyConfig& proxy_config) {
    proxy_config_ = proxy_config;
    SetProxy(proxy_config.host, proxy_config.port);
    ESP_LOGI(TAG, "Proxy configured: %s:%d", 
             proxy_config_.IsValid() ? proxy_config_.host.c_str() : "none", 
             proxy_config_.port);
}

void UniversalHttpClient::SetTimeout(int timeout_ms) {
    timeout_ms_ = timeout_ms;
    ESP_LOGV(TAG, "Setting timeout to %d ms", timeout_ms);
}

void UniversalHttpClient::SetHeader(const std::string& key, const std::string& value) {
    headers_[key] = value;
    ESP_LOGV(TAG, "Setting header: %s=%s", key.c_str(), value.c_str());
}

void UniversalHttpClient::SetContent(std::string&& content) {
    content_ = std::make_optional(std::move(content));
    ESP_LOGV(TAG, "Setting content with size: %d", content_.has_value() ? (int)content_.value().size() : 0);
}

bool UniversalHttpClient::ParseUrl(const std::string& url) {
    ESP_LOGV(TAG, "Parsing URL: %s", url.c_str());
    
    // 解析协议
    size_t protocol_end = url.find("://");
    if (protocol_end == std::string::npos) {
        ESP_LOGE(TAG, "Invalid URL format - missing protocol");
        return false;
    }
    
    std::string protocol = url.substr(0, protocol_end);
    is_https_ = (protocol == "https");
    port_ = is_https_ ? 443 : 80;
    
    // 解析主机和路径
    size_t host_start = protocol_end + 3;
    size_t host_end = url.find('/', host_start);
    
    std::string host_part;
    if (host_end != std::string::npos) {
        host_part = url.substr(host_start, host_end - host_start);
        path_ = url.substr(host_end);
    } else {
        host_part = url.substr(host_start);
        path_ = "/";
    }
    
    // 检查是否有端口号
    size_t port_pos = host_part.find(':');
    if (port_pos != std::string::npos) {
        host_ = host_part.substr(0, port_pos);
        port_ = std::stoi(host_part.substr(port_pos + 1));
    } else {
        host_ = host_part;
    }
    
    ESP_LOGV(TAG, "Parsed URL - Host: %s, Port: %d, Path: %s, HTTPS: %s", 
             host_.c_str(), port_, path_.c_str(), is_https_ ? "true" : "false");
    return true;
}

bool UniversalHttpClient::Open(const std::string& method, const std::string& url) {
    ESP_LOGI(TAG, "Opening HTTP connection - Method: %s, URL: %s", method.c_str(), url.c_str());
    
    method_ = method;
    url_ = url;
    
    // 解析URL
    if (!ParseUrl(url)) {
        ESP_LOGE(TAG, "Failed to parse URL");
        return false;
    }
    
    // 确定实际连接的主机和端口（考虑代理）
    std::string connect_host = host_;
    int connect_port = port_;
    
    if (proxy_config_.IsValid()) {
        connect_host = proxy_config_.host;
        connect_port = proxy_config_.port;
        ESP_LOGI(TAG, "Using proxy for connection: %s:%d", connect_host.c_str(), connect_port);
    }
    
    // 创建TCP连接
    if (is_https_ && !proxy_config_.IsValid()) {
        ESP_LOGI(TAG, "Creating SSL connection to %s:%d", connect_host.c_str(), connect_port);
        tcp_client_ = network_interface_->CreateSsl(0);
    } else {
        ESP_LOGI(TAG, "Creating TCP connection to %s:%d", connect_host.c_str(), connect_port);
        tcp_client_ = network_interface_->CreateTcp(0);
    }
    
    if (!tcp_client_) {
        ESP_LOGE(TAG, "Failed to create TCP client");
        return false;
    }
    
    // 设置超时
    // TCP连接本身没有直接的超时设置方法，我们将在连接时使用轮询检查超时
    
    // 连接到服务器
    ESP_LOGI(TAG, "Connecting to %s:%d", connect_host.c_str(), connect_port);
    if (!tcp_client_->Connect(connect_host, connect_port)) {
        ESP_LOGE(TAG, "Failed to connect to server");
        tcp_client_.reset();
        return false;
    }
    
    ESP_LOGI(TAG, "Connected to server %s:%d", connect_host.c_str(), connect_port);
    
    // 设置数据接收回调
    {
        std::lock_guard<std::mutex> lock(response_mutex_);
        received_data_.clear();
        headers_received_ = false;
        response_complete_ = false;
    }
    
    tcp_client_->OnStream([this](const std::string& data) {
        std::lock_guard<std::mutex> lock(response_mutex_);
        received_data_ += data;
        response_cv_.notify_one();
        ESP_LOGV(TAG, "Received %d bytes of data", (int)data.size());
    });
    
    tcp_client_->OnDisconnected([this]() {
        std::lock_guard<std::mutex> lock(response_mutex_);
        response_complete_ = true;
        response_cv_.notify_one();
        ESP_LOGV(TAG, "TCP connection disconnected");
    });
    
    // 发送HTTP请求
    if (!SendHttpRequest()) {
        ESP_LOGE(TAG, "Failed to send HTTP request");
        Close();
        return false;
    }
    
    // 等待并处理HTTP响应
    if (!WaitForResponse()) {
        ESP_LOGE(TAG, "Failed to receive HTTP response");
        Close();
        return false;
    }
    
    ESP_LOGI(TAG, "HTTP request completed successfully");
    return true;
}

std::string UniversalHttpClient::BuildHttpRequest() {
    ESP_LOGV(TAG, "Building HTTP request");
    
    std::string request = method_ + " ";
    
    // 如果使用代理，使用完整URL，否则只使用路径
    if (proxy_config_.IsValid()) {
        request += (is_https_ ? "https://" : "http://") + host_ + ":" + std::to_string(port_) + path_;
    } else {
        request += path_;
    }
    
    request += " HTTP/1.1\r\n";
    
    // 添加Host头
    if (headers_.find("Host") == headers_.end()) {
        request += "Host: " + host_ + "\r\n";
    }
    
    // 添加其他头
    for (const auto& header : headers_) {
        request += header.first + ": " + header.second + "\r\n";
    }
    
    // 添加Content-Length头（如果有内容）
    if (content_.has_value()) {
        request += "Content-Length: " + std::to_string(content_.value().size()) + "\r\n";
    }
    
    // 结束头部分
    request += "\r\n";
    
    // 添加内容（如果有）
    if (content_.has_value()) {
        request += content_.value();
    }
    
    ESP_LOGV(TAG, "Built HTTP request:\n%s", request.c_str());
    return request;
}

bool UniversalHttpClient::SendHttpRequest() {
    ESP_LOGV(TAG, "Sending HTTP request");
    
    std::string request = BuildHttpRequest();
    
    // 发送请求
    int sent = tcp_client_->Send(request);
    if (sent < 0) {
        ESP_LOGE(TAG, "Failed to send HTTP request");
        return false;
    }
    
    ESP_LOGV(TAG, "Sent %d bytes of HTTP request", sent);
    return true;
}

bool UniversalHttpClient::WaitForResponse() {
    ESP_LOGV(TAG, "Waiting for HTTP response");
    
    std::unique_lock<std::mutex> lock(response_mutex_);
    auto start_time = std::chrono::steady_clock::now();
    
    // 等待直到收到完整响应或超时
    while (!response_complete_) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
        
        if (elapsed > timeout_ms_) {
            ESP_LOGE(TAG, "HTTP response timeout");
            return false;
        }
        
        // 等待数据到达或超时
        response_cv_.wait_for(lock, std::chrono::milliseconds(100), [this] {
            return !received_data_.empty() || response_complete_;
        });
        
        // 检查是否收到了足够的数据
        if (!received_data_.empty()) {
            // 检查是否已经解析了头部
            if (!headers_received_) {
                size_t header_end = received_data_.find("\r\n\r\n");
                if (header_end != std::string::npos) {
                    headers_received_ = true;
                    std::string headers_part = received_data_.substr(0, header_end);
                    
                    if (!ParseHttpResponse(headers_part)) {
                        ESP_LOGE(TAG, "Failed to parse HTTP response headers");
                        return false;
                    }
                    
                    // 从接收数据中移除头部部分
                    response_body_ = received_data_.substr(header_end + 4);
                    received_data_.clear();
                    
                    // 检查是否是chunked传输编码
                    auto it = response_headers_.find("Transfer-Encoding");
                    if (it != response_headers_.end() && it->second.find("chunked") != std::string::npos) {
                        // 对于chunked传输，我们需要特殊处理
                        ESP_LOGW(TAG, "Chunked transfer encoding detected - may not be fully supported");
                    } else {
                        // 检查Content-Length
                        auto length_it = response_headers_.find("Content-Length");
                        if (length_it != response_headers_.end()) {
                            content_length_ = std::stoul(length_it->second);
                            response_complete_ = response_body_.size() >= content_length_;
                        } else {
                            // 没有Content-Length，我们可能需要等待连接关闭
                            // 这是一个简化处理，实际应用中可能需要更复杂的逻辑
                        }
                    }
                }
            } else {
                // 已经解析了头部，继续收集响应体
                response_body_ += received_data_;
                received_data_.clear();
                
                if (content_length_ > 0) {
                    response_complete_ = response_body_.size() >= content_length_;
                }
            }
        }
    }
    
    ESP_LOGI(TAG, "Received HTTP response - Status: %d, Body size: %d", status_code_, (int)response_body_.size());
    return true;
}

bool UniversalHttpClient::ParseHttpResponse(const std::string& response) {
    ESP_LOGV(TAG, "Parsing HTTP response headers");
    
    std::istringstream iss(response);
    std::string line;
    
    // 解析状态行
    if (std::getline(iss, line)) {
        // 移除可能的回车符
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        // 解析 "HTTP/1.1 200 OK" 格式
        size_t first_space = line.find(' ');
        if (first_space != std::string::npos) {
            size_t second_space = line.find(' ', first_space + 1);
            if (second_space != std::string::npos) {
                try {
                    status_code_ = std::stoi(line.substr(first_space + 1, second_space - first_space - 1));
                    ESP_LOGV(TAG, "Status code: %d", status_code_);
                } catch (const std::exception& e) {
                    ESP_LOGE(TAG, "Failed to parse status code: %s", e.what());
                    return false;
                }
            }
        }
    }
    
    // 解析头部
    while (std::getline(iss, line)) {
        // 移除可能的回车符
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        if (line.empty()) {
            break; // 头部结束
        }
        
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            
            // 去除前导空格
            value.erase(0, value.find_first_not_of(' '));
            
            // 转换键为标准格式（首字母大写，连字符后大写）
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            if (!key.empty()) {
                key[0] = std::toupper(key[0]);
                for (size_t i = 1; i < key.length(); ++i) {
                    if (key[i - 1] == '-') {
                        key[i] = std::toupper(key[i]);
                    }
                }
            }
            
            response_headers_[key] = value;
            ESP_LOGV(TAG, "Parsed header: %s=%s", key.c_str(), value.c_str());
        }
    }
    
    // 查找Content-Length
    auto it = response_headers_.find("Content-Length");
    if (it != response_headers_.end()) {
        try {
            content_length_ = std::stoul(it->second);
            ESP_LOGV(TAG, "Content-Length: %d", (int)content_length_);
        } catch (const std::exception& e) {
            ESP_LOGW(TAG, "Failed to parse Content-Length: %s", e.what());
        }
    }
    
    return true;
}

void UniversalHttpClient::Close() {
    ESP_LOGV(TAG, "Closing HTTP connection");
    if (tcp_client_) {
        tcp_client_->Disconnect();
        tcp_client_.reset();
    }
    
    // 通知可能正在等待的线程
    {
        std::lock_guard<std::mutex> lock(response_mutex_);
        response_complete_ = true;
    }
    response_cv_.notify_all();
}

int UniversalHttpClient::Read(char* buffer, size_t buffer_size) {
    ESP_LOGV(TAG, "Reading HTTP response, buffer size: %d", (int)buffer_size);
    
    if (response_body_.empty()) {
        ESP_LOGW(TAG, "No response body available");
        return 0;
    }
    
    size_t bytes_to_read = std::min(response_body_.size(), buffer_size);
    std::memcpy(buffer, response_body_.data(), bytes_to_read);
    
    // 从响应体中移除已读取的数据
    response_body_.erase(0, bytes_to_read);
    
    ESP_LOGV(TAG, "Read %d bytes from HTTP response", (int)bytes_to_read);
    return static_cast<int>(bytes_to_read);
}

int UniversalHttpClient::Write(const char* buffer, size_t buffer_size) {
    ESP_LOGV(TAG, "Writing HTTP request, buffer size: %d", (int)buffer_size);
    // 在Open之后不应该再写入数据
    ESP_LOGW(TAG, "Write called after Open - this is not supported in this implementation");
    return -1;
}

int UniversalHttpClient::GetStatusCode() {
    ESP_LOGV(TAG, "Getting HTTP status code: %d", status_code_);
    return status_code_;
}

std::string UniversalHttpClient::GetResponseHeader(const std::string& key) const {
    ESP_LOGV(TAG, "Getting response header: %s", key.c_str());
    
    // 查找时不区分大小写
    std::string lower_key = key;
    std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);
    
    for (const auto& header : response_headers_) {
        std::string header_key = header.first;
        std::transform(header_key.begin(), header_key.end(), header_key.begin(), ::tolower);
        
        if (header_key == lower_key) {
            return header.second;
        }
    }
    
    return "";
}

size_t UniversalHttpClient::GetBodyLength() {
    ESP_LOGV(TAG, "Getting HTTP body length: %d", (int)content_length_);
    return content_length_;
}

std::string UniversalHttpClient::ReadAll() {
    ESP_LOGV(TAG, "Reading all HTTP response data, size: %d", (int)response_body_.size());
    std::string result = response_body_;
    response_body_.clear(); // 清空已读取的数据
    return result;
}