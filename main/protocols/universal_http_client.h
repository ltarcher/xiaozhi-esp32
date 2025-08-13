#ifndef UNIVERSAL_HTTP_CLIENT_H
#define UNIVERSAL_HTTP_CLIENT_H

#include <http.h>
#include <network_interface.h>
#include "bijie_coins.h"

#include <memory>
#include <string>
#include <map>
#include <optional>

// 包含ESP-IDF HTTP客户端头文件
#include <esp_http_client.h>

class UniversalHttpClient : public Http {
public:
    explicit UniversalHttpClient(NetworkInterface* network_interface);
    ~UniversalHttpClient() override;

    // Http接口实现
    void SetTimeout(int timeout_ms) override;
    void SetHeader(const std::string& key, const std::string& value) override;
    void SetContent(std::string&& content) override;
    bool Open(const std::string& method, const std::string& url) override;
    void Close() override;
    int Read(char* buffer, size_t buffer_size) override;
    int Write(const char* buffer, size_t buffer_size) override;
    int GetStatusCode() override;
    std::string GetResponseHeader(const std::string& key) const override;
    size_t GetBodyLength() override;
    std::string ReadAll() override;
    
    // 重写基类的SetProxy方法
    void SetProxy(const std::string& host, int port);
    
    // 提供额外的代理设置方法，接受ProxyConfig结构
    void SetProxy(const ProxyConfig& proxy_config);

private:
    NetworkInterface* network_interface_;
    esp_http_client_handle_t http_client_;
    ProxyConfig proxy_config_;
    
    // HTTP请求相关
    std::string method_;
    std::string url_;
    
    // 配置设置
    int timeout_ms_ = 30000;
    std::map<std::string, std::string> headers_;
    std::optional<std::string> content_ = std::nullopt;
    
    // 响应相关
    int status_code_ = -1;
    std::map<std::string, std::string> response_headers_;
    std::string response_body_;
    size_t content_length_ = 0;
    
    
    // 内部方法
    bool ParseUrl(const std::string& url);
    esp_http_client_method_t GetMethod(const std::string& method);
};

#endif // UNIVERSAL_HTTP_CLIENT_H