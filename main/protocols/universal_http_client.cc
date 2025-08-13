#include "universal_http_client.h"
#include <esp_log.h>

static const char* TAG = "UniversalHttpClient";

UniversalHttpClient::UniversalHttpClient(NetworkInterface* network_interface) 
    : network_interface_(network_interface) {
    // 创建底层HTTP实现
    http_impl_ = std::unique_ptr<Http>(network_interface_->CreateHttp(0));
}

UniversalHttpClient::~UniversalHttpClient() {
    Close();
}

void UniversalHttpClient::SetProxy(const ProxyConfig& proxy_config) {
    proxy_config_ = proxy_config;
    
    // 如果底层HTTP实现支持代理，则设置代理
    if (http_impl_ && proxy_config_.IsValid()) {
        http_impl_->SetProxy(proxy_config_.host, proxy_config_.port);
        ESP_LOGI(TAG, "Proxy set: %s:%d", proxy_config_.host.c_str(), proxy_config_.port);
    }
}

void UniversalHttpClient::SetTimeout(int timeout_ms) {
    timeout_ms_ = timeout_ms;
    if (http_impl_) {
        http_impl_->SetTimeout(timeout_ms);
    }
}

void UniversalHttpClient::SetHeader(const std::string& key, const std::string& value) {
    headers_[key] = value;
    if (http_impl_) {
        http_impl_->SetHeader(key, value);
    }
}

void UniversalHttpClient::SetContent(std::string&& content) {
    content_ = std::make_optional(std::move(content));
    if (http_impl_) {
        http_impl_->SetContent(std::move(content));
    }
}

bool UniversalHttpClient::Open(const std::string& method, const std::string& url) {
    if (!http_impl_) {
        ESP_LOGE(TAG, "HTTP implementation is not available");
        return false;
    }

    // 应用缓存的设置
    for (const auto& header : headers_) {
        http_impl_->SetHeader(header.first, header.second);
    }
    
    if (content_.has_value()) {
        http_impl_->SetContent(std::move(content_.value()));
    }

    return http_impl_->Open(method, url);
}

void UniversalHttpClient::Close() {
    if (http_impl_) {
        http_impl_->Close();
    }
}

int UniversalHttpClient::Read(char* buffer, size_t buffer_size) {
    if (!http_impl_) {
        return -1;
    }
    return http_impl_->Read(buffer, buffer_size);
}

int UniversalHttpClient::Write(const char* buffer, size_t buffer_size) {
    if (!http_impl_) {
        return -1;
    }
    return http_impl_->Write(buffer, buffer_size);
}

int UniversalHttpClient::GetStatusCode() {
    if (!http_impl_) {
        return -1;
    }
    return http_impl_->GetStatusCode();
}

std::string UniversalHttpClient::GetResponseHeader(const std::string& key) const {
    if (!http_impl_) {
        return "";
    }
    return http_impl_->GetResponseHeader(key);
}

size_t UniversalHttpClient::GetBodyLength() {
    if (!http_impl_) {
        return 0;
    }
    return http_impl_->GetBodyLength();
}

std::string UniversalHttpClient::ReadAll() {
    if (!http_impl_) {
        return "";
    }
    return http_impl_->ReadAll();
}