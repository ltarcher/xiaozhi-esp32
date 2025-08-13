#include "bijie_coins.h"
#include <esp_log.h>
#include <cJSON.h>
#include <map>
#include <vector>
#include <memory>
#include <algorithm>
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <esp_http_client.h>
#include <esp_tls.h>

static const char* TAG = "BiJieCoins";

// 重连间隔时间（毫秒）
static const int RECONNECT_INTERVAL_MS = 5000; // 5秒

// 为每个WebSocket连接创建一个独立的实现类
class BiJieCoinConnection {
public:
    BiJieCoinConnection(int currency_id, OnMarketDataCallback callback, const ProxyConfig& proxy_config) 
        : currency_id_(currency_id), callback_(callback), reconnect_timer_(nullptr), proxy_config_(proxy_config) {
    }

    ~BiJieCoinConnection() {
        Disconnect();
    }

    bool Connect() {
        // 构造WebSocket URL
        std::string url = "wss://api.528btc.com.cn/xhj-gather-app/open/websocket/candle1m-" + std::to_string(currency_id_);
        
        ESP_LOGI(TAG, "Connecting to BiJie WebSocket for currency %d: %s", currency_id_, url.c_str());
        
        // 创建WebSocket连接
        websocket_ = std::make_unique<UniversalWebSocket>();
        
        // 设置代理（如果配置了代理）
        if (proxy_config_.IsValid()) {
            WebSocketProxyConfig ws_proxy;
            ws_proxy.host = proxy_config_.host;
            ws_proxy.port = proxy_config_.port;
            ws_proxy.username = proxy_config_.username;
            ws_proxy.password = proxy_config_.password;
#if defined(WEBSOCKET_HAS_SET_PROXY)
            websocket_->SetProxy(ws_proxy);
#else
            ESP_LOGW(TAG, "WebSocket implementation does not support proxy, proxy settings will be ignored");
#endif
        }
        
        // 设置回调函数
        SetupCallbacks();
        
        // 连接WebSocket服务器
        return websocket_->Connect(url);
    }

    void Disconnect() {
        // 删除重连定时器
        if (reconnect_timer_) {
            TimerHandle_t timer = static_cast<TimerHandle_t>(reconnect_timer_);
            xTimerDelete(timer, 0);
            reconnect_timer_ = nullptr;
        }
        
        if (websocket_) {
            websocket_->Disconnect();
            websocket_.reset();
        }
    }

    bool IsConnected() const {
        return websocket_ && websocket_->IsConnected();
    }

    int GetCurrencyId() const {
        return currency_id_;
    }

    std::shared_ptr<CoinMarketData> GetMarketData() const {
        return market_data_;
    }

private:
    void SetupCallbacks() {
        if (!websocket_) return;

        // 设置连接回调
        websocket_->OnConnected([this]() {
            ESP_LOGI(TAG, "BiJie WebSocket connected for currency %d", currency_id_);
        });

        // 设置断开连接回调
        websocket_->OnDisconnected([this]() {
            ESP_LOGI(TAG, "BiJie WebSocket disconnected for currency %d", currency_id_);
            // 启动重连定时器
            StartReconnectTimer();
        });

        // 设置错误回调
        websocket_->OnError([this](const std::string& error) {
            ESP_LOGE(TAG, "BiJie WebSocket error for currency %d: %s", currency_id_, error.c_str());
            // 启动重连定时器
            StartReconnectTimer();
        });

        // 设置数据接收回调
        websocket_->OnData([this](const uint8_t* data, size_t length, bool is_binary) {
            if (!is_binary) {
                std::string message(reinterpret_cast<const char*>(data), length);
                HandleIncomingMessage(message);
            }
        });
    }

    void StartReconnectTimer() {
        // 如果定时器已存在，先删除
        if (reconnect_timer_) {
            TimerHandle_t timer = static_cast<TimerHandle_t>(reconnect_timer_);
            xTimerDelete(timer, 0);
            reconnect_timer_ = nullptr;
        }
        
        // 创建新的重连定时器
        TimerHandle_t timer = xTimerCreate(
            "reconnect_timer",
            pdMS_TO_TICKS(RECONNECT_INTERVAL_MS),
            pdFALSE, // 不重复
            this,
            ReconnectTimerCallback
        );
        
        if (timer) {
            reconnect_timer_ = static_cast<void*>(timer);
            xTimerStart(timer, 0);
            ESP_LOGI(TAG, "Started reconnect timer for currency %d", currency_id_);
        } else {
            ESP_LOGE(TAG, "Failed to create reconnect timer for currency %d", currency_id_);
        }
    }

    static void ReconnectTimerCallback(TimerHandle_t xTimer) {
        BiJieCoinConnection* self = static_cast<BiJieCoinConnection*>(pvTimerGetTimerID(xTimer));
        
        // 删除定时器
        xTimerDelete(xTimer, 0);
        self->reconnect_timer_ = nullptr;
        
        // 尝试重新连接
        ESP_LOGI(TAG, "Attempting to reconnect WebSocket for currency %d", self->currency_id_);
        if (!self->Connect()) {
            ESP_LOGE(TAG, "Failed to reconnect WebSocket for currency %d", self->currency_id_);
            // 如果重连失败，再次启动重连定时器
            self->StartReconnectTimer();
        }
    }

    void HandleIncomingMessage(const std::string& message) {
        cJSON* root = cJSON_Parse(message.c_str());
        if (!root) {
            ESP_LOGE(TAG, "Failed to parse JSON message for currency %d", currency_id_);
            return;
        }

        // 解析行情数据
        auto market_data = std::make_shared<CoinMarketData>();
        market_data->currency_id = currency_id_;
        
        cJSON* timestamp = cJSON_GetObjectItem(root, "timestamp");
        if (cJSON_IsNumber(timestamp)) {
            market_data->timestamp = static_cast<long long>(timestamp->valuedouble);
        }
        
        cJSON* open = cJSON_GetObjectItem(root, "open");
        if (cJSON_IsNumber(open)) {
            market_data->open = static_cast<float>(open->valuedouble);
        }
        
        cJSON* high = cJSON_GetObjectItem(root, "high");
        if (cJSON_IsNumber(high)) {
            market_data->high = static_cast<float>(high->valuedouble);
        }
        
        cJSON* low = cJSON_GetObjectItem(root, "low");
        if (cJSON_IsNumber(low)) {
            market_data->low = static_cast<float>(low->valuedouble);
        }
        
        cJSON* close = cJSON_GetObjectItem(root, "close");
        if (cJSON_IsNumber(close)) {
            market_data->close = static_cast<float>(close->valuedouble);
        }
        
        cJSON* turnover = cJSON_GetObjectItem(root, "turnover");
        if (cJSON_IsNumber(turnover)) {
            market_data->turnover = static_cast<float>(turnover->valuedouble);
        }
        
        cJSON* change = cJSON_GetObjectItem(root, "change");
        if (cJSON_IsNumber(change)) {
            market_data->change = static_cast<float>(change->valuedouble);
        }
        
        cJSON* change24h = cJSON_GetObjectItem(root, "change24h");
        if (cJSON_IsNumber(change24h)) {
            market_data->change_24h = static_cast<float>(change24h->valuedouble);
        }
        
        cJSON* change7d = cJSON_GetObjectItem(root, "change7d");
        if (cJSON_IsNumber(change7d)) {
            market_data->change_7d = static_cast<float>(change7d->valuedouble);
        }
        
        cJSON* change30d = cJSON_GetObjectItem(root, "change30d");
        if (cJSON_IsNumber(change30d)) {
            market_data->change_30d = static_cast<float>(change30d->valuedouble);
        }
        
        cJSON* change1h = cJSON_GetObjectItem(root, "change1h");
        if (cJSON_IsNumber(change1h)) {
            market_data->change_1h = static_cast<float>(change1h->valuedouble);
        }
        
        cJSON* changeYear = cJSON_GetObjectItem(root, "changeYear");
        if (cJSON_IsNumber(changeYear)) {
            market_data->change_year = static_cast<float>(changeYear->valuedouble);
        }
        
        cJSON* changeNow = cJSON_GetObjectItem(root, "changeNow");
        if (cJSON_IsNumber(changeNow)) {
            market_data->change_now = static_cast<float>(changeNow->valuedouble);
        }
        
        cJSON* circulationMarket = cJSON_GetObjectItem(root, "circulationMarket");
        if (cJSON_IsNumber(circulationMarket)) {
            market_data->circulation_market = circulationMarket->valuedouble;
        }
        
        // 存储行情数据
        market_data_ = market_data;
        
        // 调用回调函数
        if (callback_) {
            callback_(*market_data);
        }
        
        cJSON_Delete(root);
    }

private:
    int currency_id_;
    std::unique_ptr<UniversalWebSocket> websocket_;
    std::shared_ptr<CoinMarketData> market_data_;
    OnMarketDataCallback callback_;
    void* reconnect_timer_; // 重连定时器
    ProxyConfig proxy_config_; // 代理配置
};

// 实现类，隐藏具体实现细节
class BiJieCoins::Impl {
public:
    Impl(BiJieCoins* parent) : parent_(parent) {
        // 初始化支持的货币ID列表
        supported_currency_ids_ = {1, 2, 17, 4, 5, 6, 7, 8, 14, 23}; // BTC, ETH, LTC, BNB, XRP, ADA, SOL, DOGE, TRX, XLM
    }

    ~Impl() {
        DisconnectAll();
    }

    void SetProxy(const ProxyConfig& proxy) {
        proxy_config_ = proxy;
    }

    ProxyConfig GetProxy() const {
        return proxy_config_;
    }

    bool Connect(int currency_id) {
        // 检查是否已经存在该货币的连接
        auto it = connections_.find(currency_id);
        if (it != connections_.end()) {
            // 如果已存在连接，先断开
            it->second->Disconnect();
        } else {
            // 创建新的连接
            auto connection = std::make_shared<BiJieCoinConnection>(currency_id, market_data_callback_, proxy_config_);
            connections_[currency_id] = connection;
        }
        
        // 连接
        return connections_[currency_id]->Connect();
    }

    void Disconnect(int currency_id) {
        auto it = connections_.find(currency_id);
        if (it != connections_.end()) {
            it->second->Disconnect();
            connections_.erase(it);
        }
    }

    void DisconnectAll() {
        for (auto& pair : connections_) {
            pair.second->Disconnect();
        }
        connections_.clear();
    }

    bool IsConnected(int currency_id) const {
        auto it = connections_.find(currency_id);
        if (it != connections_.end()) {
            return it->second->IsConnected();
        }
        return false;
    }

    std::shared_ptr<CoinMarketData> GetMarketData(int currency_id) const {
        auto it = connections_.find(currency_id);
        if (it != connections_.end()) {
            return it->second->GetMarketData();
        }
        return nullptr;
    }

    std::vector<CoinInfo> GetCoinList() const {
        std::vector<CoinInfo> coin_list;
        
        // 根据支持的货币ID构建货币列表
        for (int id : supported_currency_ids_) {
            CoinInfo coin_info;
            coin_info.id = id;
            
            // 设置货币符号和名称
            switch (id) {
                case 1:
                    coin_info.symbol = "BTC";
                    coin_info.name = "Bitcoin";
                    break;
                case 2:
                    coin_info.symbol = "ETH";
                    coin_info.name = "Ethereum";
                    break;
                case 17:
                    coin_info.symbol = "LTC";
                    coin_info.name = "Litecoin";
                    break;
                case 4:
                    coin_info.symbol = "BNB";
                    coin_info.name = "Binance Coin";
                    break;
                case 5:
                    coin_info.symbol = "XRP";
                    coin_info.name = "Ripple";
                    break;
                case 6:
                    coin_info.symbol = "ADA";
                    coin_info.name = "Cardano";
                    break;
                case 7:
                    coin_info.symbol = "SOL";
                    coin_info.name = "Solana";
                    break;
                case 8:
                    coin_info.symbol = "DOGE";
                    coin_info.name = "Dogecoin";
                    break;
                case 14:
                    coin_info.symbol = "TRX";
                    coin_info.name = "Tron";
                    break;
                case 23:
                    coin_info.symbol = "XLM";
                    coin_info.name = "Stellar";
                    break;
                default:
                    coin_info.symbol = "UNK";
                    coin_info.name = "Unknown";
                    break;
            }
            
            // 获取最新价格和变化数据
            auto market_data = GetMarketData(id);
            if (market_data) {
                coin_info.price = market_data->close;
                coin_info.change_24h = market_data->change_24h;
            } else {
                coin_info.price = 0.0f;
                coin_info.change_24h = 0.0f;
            }
            
            coin_list.push_back(coin_info);
        }
        
        return coin_list;
    }

    void SetMarketDataCallback(OnMarketDataCallback callback) {
        market_data_callback_ = std::move(callback);
    }

    void SetCoinListCallback(OnCoinListCallback callback) {
        coin_list_callback_ = std::move(callback);
    }

    std::vector<int> GetSupportedCurrencyIds() const {
        return supported_currency_ids_;
    }
    
    void GetKLineData(int currency_id, int kline_type, int limit, OnKLineDataCallback callback) {
        // 创建一个新的任务来处理HTTP请求
        auto task_data = new KLineTaskData{currency_id, kline_type, limit, callback, proxy_config_};
        
        // 创建任务处理K线数据获取
        xTaskCreatePinnedToCore(
            KLineDataTask, 
            "kline_task", 
            4096, 
            task_data, 
            5, 
            nullptr, 
            1
        );
    }

private:
    struct KLineTaskData {
        int currency_id;
        int kline_type;
        int limit;
        OnKLineDataCallback callback;
        ProxyConfig proxy_config;
    };
    
    static void KLineDataTask(void* pvParameters) {
        KLineTaskData* task_data = static_cast<KLineTaskData*>(pvParameters);
        
        std::vector<KLineData> kline_data;
        
        // 构建URL
        std::string url = "https://www.528btc.com/e/extend/api/index.php";
        url += "?m=kline&c=coin&id=" + std::to_string(task_data->currency_id);
        url += "&type=" + std::to_string(task_data->kline_type);
        url += "&limit=" + std::to_string(task_data->limit);
        url += "&sortByDate=true&sortByDateRule=false";
        
        // 根据货币ID设置符号
        std::string symbol;
        switch (task_data->currency_id) {
            case 1: symbol = "BTC"; break;
            case 2: symbol = "ETH"; break;
            case 17: symbol = "LTC"; break;
            case 4: symbol = "BNB"; break;
            case 5: symbol = "XRP"; break;
            case 6: symbol = "ADA"; break;
            case 7: symbol = "SOL"; break;
            case 8: symbol = "DOGE"; break;
            case 14: symbol = "TRX"; break;
            case 23: symbol = "XLM"; break;
            default: symbol = "UNK"; break;
        }
        url += "&symbol=" + symbol;
        
        ESP_LOGI(TAG, "Fetching K-line data from: %s", url.c_str());
        
        // 配置HTTP客户端
        esp_http_client_config_t config = {
            .url = url.c_str(),
            .method = HTTP_METHOD_POST,
            .timeout_ms = 10000,
        };
        
        esp_http_client_handle_t client = esp_http_client_init(&config);
        
        // 设置代理（如果配置了代理）
        if (task_data->proxy_config.IsValid()) {
            ESP_LOGI(TAG, "Setting HTTP proxy: %s:%d", task_data->proxy_config.host.c_str(), task_data->proxy_config.port);
#if defined(HTTP_CLIENT_HAS_SET_PROXY)
            esp_http_client_set_proxy(client, task_data->proxy_config.host.c_str(), task_data->proxy_config.port);
#else
            ESP_LOGW(TAG, "HTTP client implementation does not support proxy, proxy settings will be ignored");
#endif
        }
        
        // 设置请求头
        esp_http_client_set_header(client, "Accept", "application/json, text/javascript, */*; q=0.01");
        esp_http_client_set_header(client, "Accept-Language", "zh-CN,zh;q=0.9");
        esp_http_client_set_header(client, "Cache-Control", "no-cache");
        esp_http_client_set_header(client, "Origin", "https://www.528btc.com");
        esp_http_client_set_header(client, "Pragma", "no-cache");
        esp_http_client_set_header(client, "Referer", "https://www.528btc.com/coin/3008/trend-all");
        esp_http_client_set_header(client, "Sec-Ch-Ua", "\"Not)A;Brand\";v=\"8\", \"Chromium\";v=\"138\", \"Google Chrome\";v=\"138\"");
        esp_http_client_set_header(client, "Sec-Ch-Ua-Mobile", "?0");
        esp_http_client_set_header(client, "Sec-Ch-Ua-Platform", "\"Windows\"");
        esp_http_client_set_header(client, "Sec-Fetch-Dest", "empty");
        esp_http_client_set_header(client, "Sec-Fetch-Mode", "cors");
        esp_http_client_set_header(client, "Sec-Fetch-Site", "same-origin");
        esp_http_client_set_header(client, "User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/138.0.0.0 Safari/537.36");
        esp_http_client_set_header(client, "X-Requested-With", "XMLHttpRequest");
        
        // 执行请求
        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) {
            int status_code = esp_http_client_get_status_code(client);
            if (status_code == 200) {
                // 获取响应数据长度
                int content_length = esp_http_client_get_content_length(client);
                if (content_length > 0) {
                    // 分配内存存储响应数据
                    char* buffer = new char[content_length + 1];
                    int read_len = esp_http_client_read(client, buffer, content_length);
                    if (read_len > 0) {
                        buffer[read_len] = '\0';
                        ESP_LOGI(TAG, "Response: %s", buffer);
                        
                        // 解析JSON数据
                        cJSON* root = cJSON_Parse(buffer);
                        if (root) {
                            // 解析K线数据数组
                            if (cJSON_IsArray(root)) {
                                int array_size = cJSON_GetArraySize(root);
                                for (int i = 0; i < array_size; i++) {
                                    cJSON* item = cJSON_GetArrayItem(root, i);
                                    if (item) {
                                        KLineData kline_item = {0};
                                        
                                        cJSON* timestamp = cJSON_GetObjectItem(item, "T");
                                        if (cJSON_IsNumber(timestamp)) {
                                            kline_item.timestamp = static_cast<long long>(timestamp->valuedouble);
                                        }
                                        
                                        cJSON* open = cJSON_GetObjectItem(item, "o");
                                        if (cJSON_IsNumber(open)) {
                                            kline_item.open = static_cast<float>(open->valuedouble);
                                        }
                                        
                                        cJSON* high = cJSON_GetObjectItem(item, "h");
                                        if (cJSON_IsNumber(high)) {
                                            kline_item.high = static_cast<float>(high->valuedouble);
                                        }
                                        
                                        cJSON* low = cJSON_GetObjectItem(item, "l");
                                        if (cJSON_IsNumber(low)) {
                                            kline_item.low = static_cast<float>(low->valuedouble);
                                        }
                                        
                                        cJSON* close = cJSON_GetObjectItem(item, "c");
                                        if (cJSON_IsNumber(close)) {
                                            kline_item.close = static_cast<float>(close->valuedouble);
                                        }
                                        
                                        cJSON* volume = cJSON_GetObjectItem(item, "v");
                                        if (cJSON_IsNumber(volume)) {
                                            kline_item.volume = static_cast<float>(volume->valuedouble);
                                        }
                                        
                                        kline_data.push_back(kline_item);
                                    }
                                }
                            }
                            cJSON_Delete(root);
                        }
                    }
                    delete[] buffer;
                }
            } else {
                ESP_LOGE(TAG, "HTTP request failed with status code: %d", status_code);
            }
        } else {
            ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
        }
        
        // 清理HTTP客户端
        esp_http_client_cleanup(client);
        
        // 调用回调函数
        if (task_data->callback) {
            task_data->callback(kline_data);
        }
        
        // 清理任务数据
        delete task_data;
        
        // 删除任务
        vTaskDelete(nullptr);
    }

private:
    BiJieCoins* parent_;
    std::map<int, std::shared_ptr<BiJieCoinConnection>> connections_;
    std::vector<int> supported_currency_ids_;
    OnMarketDataCallback market_data_callback_;
    OnCoinListCallback coin_list_callback_;
    ProxyConfig proxy_config_; // 代理配置
};

// BiJieCoins类的实现
BiJieCoins::BiJieCoins() 
    : impl_(std::make_unique<Impl>(this)) {
}

BiJieCoins::~BiJieCoins() = default;

void BiJieCoins::SetProxy(const ProxyConfig& proxy) {
    impl_->SetProxy(proxy);
}

ProxyConfig BiJieCoins::GetProxy() const {
    return impl_->GetProxy();
}

bool BiJieCoins::Connect(int currency_id) {
    return impl_->Connect(currency_id);
}

void BiJieCoins::Disconnect(int currency_id) {
    impl_->Disconnect(currency_id);
}

void BiJieCoins::DisconnectAll() {
    impl_->DisconnectAll();
}

bool BiJieCoins::IsConnected(int currency_id) const {
    return impl_->IsConnected(currency_id);
}

std::shared_ptr<CoinMarketData> BiJieCoins::GetMarketData(int currency_id) const {
    return impl_->GetMarketData(currency_id);
}

std::vector<CoinInfo> BiJieCoins::GetCoinList() const {
    return impl_->GetCoinList();
}

void BiJieCoins::SetMarketDataCallback(OnMarketDataCallback callback) {
    impl_->SetMarketDataCallback(std::move(callback));
}

void BiJieCoins::SetCoinListCallback(OnCoinListCallback callback) {
    impl_->SetCoinListCallback(std::move(callback));
}

std::vector<int> BiJieCoins::GetSupportedCurrencyIds() const {
    return impl_->GetSupportedCurrencyIds();
}

void BiJieCoins::GetKLineData(int currency_id, int kline_type, int limit, OnKLineDataCallback callback) {
    impl_->GetKLineData(currency_id, kline_type, limit, std::move(callback));
}