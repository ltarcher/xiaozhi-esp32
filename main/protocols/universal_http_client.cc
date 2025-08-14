#include "universal_http_client.h"
#include <esp_log.h>
#include <cstring>
#include <algorithm>
#include <sstream>

static const char* TAG = "UniversalHttpClient";

// 事件处理回调函数
esp_err_t UniversalHttpClient::HttpEventHandler(esp_http_client_event_t *evt) {
    UniversalHttpClient* client = static_cast<UniversalHttpClient*>(evt->user_data);
    
    switch (evt->event_id) {
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP connected");
            break;
            
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGV(TAG, "HTTP header sent");
            break;
            
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGV(TAG, "HTTP header received: %s: %s", evt->header_key, evt->header_value);
            client->response_headers_[std::string(evt->header_key)] = std::string(evt->header_value);
            break;
            
        case HTTP_EVENT_ON_DATA:
            ESP_LOGV(TAG, "HTTP data received: %d bytes", evt->data_len);
            client->response_body_.append(static_cast<char*>(evt->data), evt->data_len);
            break;
            
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP finished");
            client->response_complete_ = true;
            break;
            
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP disconnected");
            client->response_complete_ = true;
            break;
            
        default:
            break;
    }
    
    return ESP_OK;
}

UniversalHttpClient::UniversalHttpClient(NetworkInterface* network_interface) 
    : network_interface_(network_interface), http_client_(nullptr), response_complete_(false) {
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
    
    if (http_client_) {
        esp_http_client_set_timeout_ms(http_client_, timeout_ms_);
    }
}

void UniversalHttpClient::SetHeader(const std::string& key, const std::string& value) {
    headers_[key] = value;
    ESP_LOGV(TAG, "Setting header: %s=%s", key.c_str(), value.c_str());
    
    if (http_client_) {
        esp_http_client_set_header(http_client_, key.c_str(), value.c_str());
    }
}

void UniversalHttpClient::SetContent(std::string&& content) {
    content_ = std::make_optional(std::move(content));
    ESP_LOGV(TAG, "Setting content with size: %d", content_.has_value() ? (int)content_.value().size() : 0);
}

void UniversalHttpClient::SetParam(const std::string& key, const std::string& value) {
    params_[key] = value;
    ESP_LOGV(TAG, "Setting param: %s=%s", key.c_str(), value.c_str());
}

void UniversalHttpClient::SetBody(const std::string& body) {
    body_ = std::make_optional(body);
    ESP_LOGV(TAG, "Setting body with size: %d", body_.has_value() ? (int)body_.value().size() : 0);
    
    // 如果客户端已经初始化，则更新Content-Type为application/json
    if (http_client_) {
        esp_http_client_set_header(http_client_, "Content-Type", "application/json");
    }
}

bool UniversalHttpClient::SetCookie(const std::string& name, const std::string& value) {
    cookies_[name] = value;
    ESP_LOGV(TAG, "Setting cookie: %s=%s", name.c_str(), value.c_str());
    
    // 如果客户端已经初始化，则更新Cookie头
    if (http_client_) {
        std::string cookie_header = BuildCookieHeader();
        if (!cookie_header.empty()) {
            esp_http_client_set_header(http_client_, "Cookie", cookie_header.c_str());
        }
    }
    
    return true;
}

bool UniversalHttpClient::SetCookie(const std::string& cookie) {
    // 解析cookie字符串 "name=value" 格式
    size_t equal_pos = cookie.find('=');
    if (equal_pos != std::string::npos) {
        std::string name = cookie.substr(0, equal_pos);
        std::string value = cookie.substr(equal_pos + 1);
        return SetCookie(name, value);
    } else {
        ESP_LOGW(TAG, "Invalid cookie format: %s", cookie.c_str());
        return false;
    }
}

std::string UniversalHttpClient::GetCookie(const std::string& name) const {
    auto it = cookies_.find(name);
    if (it != cookies_.end()) {
        return it->second;
    }
    return "";
}

std::string UniversalHttpClient::GetAllCookies() const {
    return BuildCookieHeader();
}

std::string UniversalHttpClient::BuildCookieHeader() const {
    if (cookies_.empty()) {
        return "";
    }
    
    std::ostringstream cookie_stream;
    bool first = true;
    for (const auto& cookie : cookies_) {
        if (!first) {
            cookie_stream << "; ";
        }
        cookie_stream << cookie.first << "=" << cookie.second;
        first = false;
    }
    
    return cookie_stream.str();
}

std::string UniversalHttpClient::BuildUrlWithParams(const std::string& base_url) const {
    if (params_.empty()) {
        return base_url;
    }
    
    std::string url = base_url;
    
    // 检查URL是否已包含查询参数
    bool has_query = base_url.find('?') != std::string::npos;
    char separator = has_query ? '&' : '?';
    
    bool first = true;
    for (const auto& param : params_) {
        if (!first || !has_query) {
            url += separator;
        }
        url += param.first + "=" + param.second;
        separator = '&';
        first = false;
    }
    
    return url;
}

esp_http_client_method_t UniversalHttpClient::GetMethod(const std::string& method) {
    std::string upper_method = method;
    std::transform(upper_method.begin(), upper_method.end(), upper_method.begin(), ::toupper);
    
    if (upper_method == "GET") {
        return HTTP_METHOD_GET;
    } else if (upper_method == "POST") {
        return HTTP_METHOD_POST;
    } else if (upper_method == "PUT") {
        return HTTP_METHOD_PUT;
    } else if (upper_method == "DELETE") {
        return HTTP_METHOD_DELETE;
    } else if (upper_method == "HEAD") {
        return HTTP_METHOD_HEAD;
    } else if (upper_method == "PATCH") {
        return HTTP_METHOD_PATCH;
    } else {
        ESP_LOGW(TAG, "Unknown HTTP method: %s, defaulting to GET", method.c_str());
        return HTTP_METHOD_GET;
    }
}

bool UniversalHttpClient::Open(const std::string& method, const std::string& url) {
    // 构建带参数的URL
    std::string final_url = BuildUrlWithParams(url);
    ESP_LOGI(TAG, "Opening HTTP connection - Method: %s, URL: %s", method.c_str(), final_url.c_str());
    
    method_ = method;
    url_ = final_url;
    
    // 先关闭之前的连接（如果有的话）
    Close();
    
    // 重置响应状态
    response_headers_.clear();
    response_body_.clear();
    response_complete_ = false;
    status_code_ = -1;
    content_length_ = 0;
    
    // 配置HTTP客户端
    esp_http_client_config_t config = {};
    config.url = final_url.c_str();
    config.method = GetMethod(method);
    config.timeout_ms = timeout_ms_;
    config.event_handler = HttpEventHandler;
    config.user_data = this;
    
    // 禁用服务器证书验证（对于自签名证书或测试环境）
    if (disable_ssl_verification_) {
        ESP_LOGI(TAG, "Disabling SSL verification");
        config.crt_bundle_attach = nullptr;
        config.use_global_ca_store = false;
    } else {
        ESP_LOGI(TAG, "Using default SSL verification");
        config.use_global_ca_store = true;
        config.crt_bundle_attach = esp_crt_bundle_attach; // 使用证书包
    }
    
    // 如果配置了代理，则设置代理
    if (proxy_config_.IsValid()) {
        ESP_LOGI(TAG, "Setting proxy: %s:%d", proxy_config_.host.c_str(), proxy_config_.port);
#if defined(HTTP_CLIENT_HAS_SET_PROXY)
        config.proxy_host = proxy_config_.host.c_str();
        config.proxy_port = proxy_config_.port;
#endif
    }
    
    // 初始化HTTP客户端
    http_client_ = esp_http_client_init(&config);
    if (!http_client_) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return false;
    }
    
    // 应用缓存的设置
    ESP_LOGV(TAG, "Applying %d cached headers", (int)headers_.size());
    for (const auto& header : headers_) {
        esp_http_client_set_header(http_client_, header.first.c_str(), header.second.c_str());
    }
    
    // 设置Cookie头
    if (!cookies_.empty()) {
        std::string cookie_header = BuildCookieHeader();
        if (!cookie_header.empty()) {
            ESP_LOGV(TAG, "Setting Cookie header: %s", cookie_header.c_str());
            esp_http_client_set_header(http_client_, "Cookie", cookie_header.c_str());
        }
    }
    
    // 设置请求内容（优先使用body_，然后是content_）
    if (body_.has_value()) {
        const std::string& body = body_.value();
        esp_http_client_set_post_field(http_client_, body.c_str(), body.length());
        // 确保Content-Type设置为application/json
        esp_http_client_set_header(http_client_, "Content-Type", "application/json");
    } else if (content_.has_value()) {
        const std::string& content = content_.value();
        esp_http_client_set_post_field(http_client_, content.c_str(), content.length());
    }
    
    // 执行HTTP请求
    esp_err_t err = esp_http_client_perform(http_client_);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
        Close();
        return false;
    }

    // 等待HTTP请求完成
    while (!response_complete_) {
        vTaskDelay(10 / portTICK_PERIOD_MS); // 简单忙等待，适当延时降低CPU使用率
    }

    // 获取状态码
    status_code_ = esp_http_client_get_status_code(http_client_);
    content_length_ = esp_http_client_get_content_length(http_client_);

    ESP_LOGI(TAG, "HTTP request completed successfully - Status: %d, Content-Length: %d", 
             status_code_, (int)content_length_);
    return true;
}

void UniversalHttpClient::Close() {
    ESP_LOGV(TAG, "Closing HTTP connection");
    if (http_client_) {
        esp_http_client_close(http_client_);
        esp_http_client_cleanup(http_client_);
        http_client_ = nullptr;
    }
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
    
    auto it = response_headers_.find(key);
    if (it != response_headers_.end()) {
        return it->second;
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