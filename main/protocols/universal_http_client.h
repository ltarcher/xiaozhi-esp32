#ifndef UNIVERSAL_HTTP_CLIENT_H
#define UNIVERSAL_HTTP_CLIENT_H

#include <http.h>
#include <network_interface.h>
#include <tcp.h>
#include "bijie_coins.h"

#include <memory>
#include <string>
#include <map>
#include <optional>
#include <mutex>
#include <condition_variable>

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
    std::unique_ptr<Tcp> tcp_client_;
    ProxyConfig proxy_config_;
    
    // HTTP请求相关
    std::string method_;
    std::string url_;
    std::string host_;
    int port_;
    std::string path_;
    bool is_https_;
    
    // 配置设置
    int timeout_ms_ = 30000;
    std::map<std::string, std::string> headers_;
    std::optional<std::string> content_ = std::nullopt;
    
    // 响应相关
    int status_code_ = -1;
    std::map<std::string, std::string> response_headers_;
    std::string response_body_;
    size_t content_length_ = 0;
    
    // 同步机制
    std::mutex response_mutex_;
    std::condition_variable response_cv_;
    bool headers_received_ = false;
    bool response_complete_ = false;
    std::string received_data_;
    
    // 内部方法
    bool ParseUrl(const std::string& url);
    std::string BuildHttpRequest();
    bool SendHttpRequest();
    bool WaitForResponse();
    bool ParseHttpResponse(const std::string& response);
    std::string EncodeBase64(const std::string& data);
};

#endif // UNIVERSAL_HTTP_CLIENT_H