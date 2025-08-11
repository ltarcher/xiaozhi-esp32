#ifndef UNIVERSAL_WEBSOCKET_H
#define UNIVERSAL_WEBSOCKET_H

#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

// 通用WebSocket类，支持ws和wss协议
class UniversalWebSocket {
public:
    enum WebSocketState {
        STATE_CLOSED = 0,
        STATE_CONNECTING = 1,
        STATE_CONNECTED = 2,
        STATE_ERROR = 3
    };

    // 回调函数类型定义
    using OnConnectedCallback = std::function<void()>;
    using OnDisconnectedCallback = std::function<void()>;
    using OnErrorCallback = std::function<void(const std::string& error)>;
    using OnDataCallback = std::function<void(const uint8_t* data, size_t length, bool is_binary)>;

    UniversalWebSocket();
    ~UniversalWebSocket();

    /**
     * @brief 连接到WebSocket服务器
     * @param url 服务器地址，支持ws://和wss://协议
     * @param headers 可选的HTTP头
     * @return true表示连接请求已发送，false表示连接失败
     */
    bool Connect(const std::string& url, const std::vector<std::pair<std::string, std::string>>& headers = {});

    /**
     * @brief 断开WebSocket连接
     */
    void Disconnect();

    /**
     * @brief 发送文本消息
     * @param message 文本消息内容
     * @return true表示发送成功，false表示发送失败
     */
    bool SendText(const std::string& message);

    /**
     * @brief 发送二进制数据
     * @param data 二进制数据指针
     * @param length 数据长度
     * @return true表示发送成功，false表示发送失败
     */
    bool SendBinary(const uint8_t* data, size_t length);

    /**
     * @brief 检查WebSocket连接状态
     * @return 当前连接状态
     */
    WebSocketState GetState() const;

    /**
     * @brief 检查WebSocket是否已连接
     * @return true表示已连接，false表示未连接
     */
    bool IsConnected() const;

    // 设置回调函数
    void OnConnected(OnConnectedCallback callback);
    void OnDisconnected(OnDisconnectedCallback callback);
    void OnError(OnErrorCallback callback);
    void OnData(OnDataCallback callback);

private:
    class Impl;  // 前向声明实现类
    std::unique_ptr<Impl> impl_;

    // 回调函数
    OnConnectedCallback on_connected_;
    OnDisconnectedCallback on_disconnected_;
    OnErrorCallback on_error_;
    OnDataCallback on_data_;
};

#endif // UNIVERSAL_WEBSOCKET_H