#include "universal_websocket.h"
#include <esp_log.h>
#include <cstring>

// 包含项目中的网络接口
#include "board.h"
#include <web_socket.h>

static const char* TAG = "UniversalWebSocket";

// 实现类，隐藏具体实现细节
class UniversalWebSocket::Impl {
public:
    Impl(UniversalWebSocket* parent) : parent_(parent), websocket_(nullptr) {
    }

    ~Impl() {
        if (websocket_) {
            // 清理WebSocket资源
            websocket_.reset();
        }
    }

    void SetProxy(const WebSocketProxyConfig& proxy) {
        proxy_config_ = proxy;
    }

    WebSocketProxyConfig GetProxy() const {
        return proxy_config_;
    }

    bool Connect(const std::string& url, const std::vector<std::pair<std::string, std::string>>& headers) {
        // 获取网络接口
        auto network = Board::GetInstance().GetNetwork();
        if (!network) {
            ESP_LOGE(TAG, "Network interface is not available");
            return false;
        }

        // 创建WebSocket连接
        websocket_ = network->CreateWebSocket(1);
        if (!websocket_) {
            ESP_LOGE(TAG, "Failed to create WebSocket");
            return false;
        }

        // 设置回调函数
        SetupCallbacks();

        // 设置HTTP头
        for (const auto& header : headers) {
            websocket_->SetHeader(header.first.c_str(), header.second.c_str());
        }

        // 设置代理（如果配置了代理）
        if (proxy_config_.IsValid()) {
            ESP_LOGI(TAG, "Setting proxy: %s:%d", proxy_config_.host.c_str(), proxy_config_.port);
            // 检查WebSocket是否支持SetProxy方法（通过编译时检查）
#if defined(WEBSOCKET_HAS_SET_PROXY)
            websocket_->SetProxy(proxy_config_.host.c_str(), proxy_config_.port);
#else
            ESP_LOGW(TAG, "WebSocket implementation does not support SetProxy method");
#endif
        }

        // 连接WebSocket服务器
        ESP_LOGI(TAG, "Connecting to WebSocket server: %s", url.c_str());
        if (!websocket_->Connect(url.c_str())) {
            ESP_LOGE(TAG, "Failed to connect to WebSocket server: %s", url.c_str());
            websocket_.reset();
            return false;
        }

        return true;
    }

    void Disconnect() {
        if (websocket_ && websocket_->IsConnected()) {
            websocket_->Close();
        }
        websocket_.reset();
    }

    bool SendText(const std::string& message) {
        if (!websocket_ || !websocket_->IsConnected()) {
            ESP_LOGE(TAG, "WebSocket is not connected");
            return false;
        }

        return websocket_->Send(message);
    }

    bool SendBinary(const uint8_t* data, size_t length) {
        if (!websocket_ || !websocket_->IsConnected()) {
            ESP_LOGE(TAG, "WebSocket is not connected");
            return false;
        }

        return websocket_->Send(reinterpret_cast<const char*>(data), length, true);
    }

    UniversalWebSocket::WebSocketState GetState() const {
        if (!websocket_) {
            return UniversalWebSocket::STATE_CLOSED;
        }

        if (websocket_->IsConnected()) {
            return UniversalWebSocket::STATE_CONNECTED;
        }

        // 可以根据具体实现添加更多状态判断
        return UniversalWebSocket::STATE_CONNECTING;
    }

private:
    void SetupCallbacks() {
        if (!websocket_) return;

        // 数据接收回调
        websocket_->OnData([this](const char* data, size_t len, bool binary) {
            if (parent_->on_data_) {
                parent_->on_data_(reinterpret_cast<const uint8_t*>(data), len, binary);
            }
        });

        // 断开连接回调
        websocket_->OnDisconnected([this]() {
            ESP_LOGI(TAG, "WebSocket disconnected");
            if (parent_->on_disconnected_) {
                parent_->on_disconnected_();
            }
        });

        // 错误回调（如果WebSocket类支持）
        // 注意：根据实际的WebSocket实现可能需要调整
    }

private:
    UniversalWebSocket* parent_;
    std::unique_ptr<WebSocket> websocket_;
    WebSocketProxyConfig proxy_config_; // 代理配置
};

// UniversalWebSocket类的实现
UniversalWebSocket::UniversalWebSocket() 
    : impl_(std::make_unique<Impl>(this)) {
}

UniversalWebSocket::~UniversalWebSocket() = default;

void UniversalWebSocket::SetProxy(const WebSocketProxyConfig& proxy) {
    impl_->SetProxy(proxy);
}

WebSocketProxyConfig UniversalWebSocket::GetProxy() const {
    return impl_->GetProxy();
}

bool UniversalWebSocket::Connect(const std::string& url, const std::vector<std::pair<std::string, std::string>>& headers) {
    return impl_->Connect(url, headers);
}

void UniversalWebSocket::Disconnect() {
    impl_->Disconnect();
}

bool UniversalWebSocket::SendText(const std::string& message) {
    return impl_->SendText(message);
}

bool UniversalWebSocket::SendBinary(const uint8_t* data, size_t length) {
    return impl_->SendBinary(data, length);
}

UniversalWebSocket::WebSocketState UniversalWebSocket::GetState() const {
    return impl_->GetState();
}

bool UniversalWebSocket::IsConnected() const {
    return GetState() == STATE_CONNECTED;
}

void UniversalWebSocket::OnConnected(OnConnectedCallback callback) {
    on_connected_ = std::move(callback);
}

void UniversalWebSocket::OnDisconnected(OnDisconnectedCallback callback) {
    on_disconnected_ = std::move(callback);
}

void UniversalWebSocket::OnError(OnErrorCallback callback) {
    on_error_ = std::move(callback);
}

void UniversalWebSocket::OnData(OnDataCallback callback) {
    on_data_ = std::move(callback);
}