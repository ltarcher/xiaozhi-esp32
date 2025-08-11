#include "universal_websocket.h"
#include "settings.h"
#include "system_info.h"
#include "board.h"
#include "assets/lang_config.h"

#include <esp_log.h>
#include <cJSON.h>
#include <cstring>

static const char* TAG = "UniversalWebSocketExample";

class UniversalWebSocketExample {
public:
    UniversalWebSocketExample() : websocket_(std::make_unique<UniversalWebSocket>()) {
        // 设置WebSocket回调
        websocket_->OnConnected([this]() {
            ESP_LOGI(TAG, "WebSocket connected");
            SendHelloMessage();
        });

        websocket_->OnDisconnected([this]() {
            ESP_LOGI(TAG, "WebSocket disconnected");
        });

        websocket_->OnError([this](const std::string& error) {
            ESP_LOGE(TAG, "WebSocket error: %s", error.c_str());
        });

        websocket_->OnData([this](const uint8_t* data, size_t length, bool is_binary) {
            if (is_binary) {
                ESP_LOGI(TAG, "Received binary data, length: %d", length);
            } else {
                std::string message(reinterpret_cast<const char*>(data), length);
                ESP_LOGI(TAG, "Received text message: %s", message.c_str());
                HandleIncomingMessage(message);
            }
        });
    }

    bool ConnectToServer() {
        Settings settings("websocket", false);
        std::string url = settings.GetString("url");
        std::string token = settings.GetString("token");
        
        if (url.empty()) {
            ESP_LOGE(TAG, "WebSocket URL is not configured");
            return false;
        }

        // 准备HTTP头
        std::vector<std::pair<std::string, std::string>> headers;
        
        // 添加认证头
        if (!token.empty()) {
            // 如果token中没有空格，添加"Bearer "前缀
            if (token.find(" ") == std::string::npos) {
                token = "Bearer " + token;
            }
            headers.push_back({"Authorization", token});
        }
        
        // 添加设备信息头
        headers.push_back({"Device-Id", SystemInfo::GetMacAddress()});
        headers.push_back({"Client-Id", Board::GetInstance().GetUuid()});
        
        ESP_LOGI(TAG, "Connecting to WebSocket server: %s", url.c_str());
        return websocket_->Connect(url, headers);
    }

    void Disconnect() {
        websocket_->Disconnect();
    }

    bool SendTextMessage(const std::string& message) {
        if (!websocket_->IsConnected()) {
            ESP_LOGE(TAG, "WebSocket is not connected");
            return false;
        }
        
        return websocket_->SendText(message);
    }

    bool SendBinaryData(const uint8_t* data, size_t length) {
        if (!websocket_->IsConnected()) {
            ESP_LOGE(TAG, "WebSocket is not connected");
            return false;
        }
        
        return websocket_->SendBinary(data, length);
    }

private:
    std::unique_ptr<UniversalWebSocket> websocket_;

    void SendHelloMessage() {
        cJSON* root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "type", "hello");
        cJSON_AddNumberToObject(root, "version", 1);
        
        cJSON* features = cJSON_CreateObject();
        cJSON_AddBoolToObject(features, "mcp", true);
        cJSON_AddItemToObject(root, "features", features);
        
        cJSON_AddStringToObject(root, "transport", "websocket");
        
        cJSON* audio_params = cJSON_CreateObject();
        cJSON_AddStringToObject(audio_params, "format", "opus");
        cJSON_AddNumberToObject(audio_params, "sample_rate", 16000);
        cJSON_AddNumberToObject(audio_params, "channels", 1);
        cJSON_AddNumberToObject(audio_params, "frame_duration", 20);
        cJSON_AddItemToObject(root, "audio_params", audio_params);
        
        char* json_str = cJSON_PrintUnformatted(root);
        std::string message(json_str);
        
        websocket_->SendText(message);
        
        cJSON_free(json_str);
        cJSON_Delete(root);
    }

    void HandleIncomingMessage(const std::string& message) {
        cJSON* root = cJSON_Parse(message.c_str());
        if (!root) {
            ESP_LOGE(TAG, "Failed to parse JSON message");
            return;
        }

        cJSON* type = cJSON_GetObjectItem(root, "type");
        if (!cJSON_IsString(type)) {
            cJSON_Delete(root);
            return;
        }

        if (strcmp(type->valuestring, "hello") == 0) {
            HandleServerHello(root);
        } else if (strcmp(type->valuestring, "response") == 0) {
            HandleServerResponse(root);
        } else {
            ESP_LOGI(TAG, "Received unknown message type: %s", type->valuestring);
        }

        cJSON_Delete(root);
    }

    void HandleServerHello(cJSON* message) {
        ESP_LOGI(TAG, "Received server hello message");
        cJSON* session_id = cJSON_GetObjectItem(message, "session_id");
        if (cJSON_IsString(session_id)) {
            ESP_LOGI(TAG, "Session ID: %s", session_id->valuestring);
        }
    }

    void HandleServerResponse(cJSON* message) {
        cJSON* status = cJSON_GetObjectItem(message, "status");
        if (cJSON_IsString(status)) {
            ESP_LOGI(TAG, "Server response status: %s", status->valuestring);
        }
    }
};

// 使用示例
extern "C" void universal_websocket_example() {
    UniversalWebSocketExample example;
    
    // 连接到服务器
    if (!example.ConnectToServer()) {
        ESP_LOGE(TAG, "Failed to connect to WebSocket server");
        return;
    }
    
    // 等待连接建立
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 发送测试消息
    if (example.SendTextMessage("{\"type\":\"test\",\"message\":\"Hello from ESP32!\"}")) {
        ESP_LOGI(TAG, "Test message sent successfully");
    } else {
        ESP_LOGE(TAG, "Failed to send test message");
    }
    
    // 保持连接一段时间以接收消息
    vTaskDelay(pdMS_TO_TICKS(10000));
    
    // 断开连接
    example.Disconnect();
}