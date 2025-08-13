#ifndef UNIVERSAL_HTTP_CLIENT_H
#define UNIVERSAL_HTTP_CLIENT_H

#include <http.h>
#include <network_interface.h>
#include "bijie_coins.h"

#include <memory>
#include <string>

class UniversalHttpClient : public Http {
public:
    explicit UniversalHttpClient(NetworkInterface* network_interface);
    ~UniversalHttpClient() override;

    // 设置代理
    void SetProxy(const ProxyConfig& proxy_config);

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

private:
    NetworkInterface* network_interface_;
    std::unique_ptr<Http> http_impl_;
    ProxyConfig proxy_config_;
    
    // 缓存设置
    int timeout_ms_ = 30000;
    std::map<std::string, std::string> headers_;
    std::optional<std::string> content_ = std::nullopt;
};

#endif // UNIVERSAL_HTTP_CLIENT_H