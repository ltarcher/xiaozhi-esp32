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
#include "board.h"
#include "universal_http_client.h"  // 使用 UniversalHttpClient
#include <set>
#include <mutex>
#include <sstream>
#include <iomanip>

// 用于标识K线请求的键值结构
struct KLineRequestKey {
    int currency_id;
    int kline_type;
    
    bool operator<(const KLineRequestKey& other) const {
        if (currency_id != other.currency_id) {
            return currency_id < other.currency_id;
        }
        return kline_type < other.kline_type;
    }
    
    bool operator==(const KLineRequestKey& other) const {
        return currency_id == other.currency_id && kline_type == other.kline_type;
    }
};

static const char* TAG = "BiJieCoins";

// 重连间隔时间（毫秒）
static const int RECONNECT_INTERVAL_MS = 5000; // 5秒
static std::set<KLineRequestKey> pending_kline_requests_; // 跟踪正在进行的K线数据请求
static std::mutex kline_requests_mutex_; // 互斥锁保护pending_kline_requests_

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
            ESP_LOGE(TAG, "Failed to parse JSON message (%s) for currency %d", message.c_str(), currency_id_);
            return;
        }
        ESP_LOGI(TAG, "Received message for currency %d: %s", currency_id_, message.c_str());

        // 解析行情数据
        auto market_data = std::make_shared<CoinMarketData>();
        
        // 如果已经有市场数据，保留已有的K线数据
        if (market_data_) {
            market_data->kline_data_1m = std::move(market_data_->kline_data_1m);
            market_data->kline_data_5m = std::move(market_data_->kline_data_5m);
            market_data->kline_data_15m = std::move(market_data_->kline_data_15m);
            market_data->kline_data_1h = std::move(market_data_->kline_data_1h);
            market_data->kline_data_2h = std::move(market_data_->kline_data_2h);
            market_data->kline_data_4h = std::move(market_data_->kline_data_4h);
            market_data->kline_data_1d = std::move(market_data_->kline_data_1d);
            market_data->kline_data_1w = std::move(market_data_->kline_data_1w);
            market_data->kline_data_1mo = std::move(market_data_->kline_data_1mo);
            market_data->kline_data_3mo = std::move(market_data_->kline_data_3mo);
        }
        
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
                    coin_info.name = "Binance-Coin";
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
        // 检查是否已经有相同currency_id和kline_type的请求正在进行
        {
            std::lock_guard<std::mutex> lock(kline_requests_mutex_);
            KLineRequestKey key{currency_id, kline_type};
            if (pending_kline_requests_.find(key) != pending_kline_requests_.end()) {
                ESP_LOGW(TAG, "K-line data request for currency %d with type %d is already in progress, returning existing data", currency_id, kline_type);
                // 如果已经有请求在进行，直接调用回调函数返回现有数据
                if (callback) {
                    // 获取现有数据
                    std::vector<KLineData> existing_data;
                    
                    // 从对应连接中获取已有的K线数据
                    auto connection_it = connections_.find(currency_id);
                    if (connection_it != connections_.end()) {
                        auto connection = connection_it->second;
                        if (connection) {
                            auto market_data = connection->GetMarketData();
                            if (market_data) {
                                // 根据kline_type获取对应的K线数据
                                const std::vector<KLineData>* kline_data_ptr = nullptr;
                                switch (kline_type) {
                                    case 13: // 1分钟
                                        kline_data_ptr = &market_data->kline_data_1m;
                                        break;
                                    case 14: // 5分钟
                                        kline_data_ptr = &market_data->kline_data_5m;
                                        break;
                                    case 1: // 15分钟
                                        kline_data_ptr = &market_data->kline_data_15m;
                                        break;
                                    case 2: // 1小时
                                        kline_data_ptr = &market_data->kline_data_1h;
                                        break;
                                    case 10: // 2小时
                                        kline_data_ptr = &market_data->kline_data_2h;
                                        break;
                                    case 11: // 4小时
                                        kline_data_ptr = &market_data->kline_data_4h;
                                        break;
                                    case 3: // 1天
                                        kline_data_ptr = &market_data->kline_data_1d;
                                        break;
                                    case 4: // 1周
                                        kline_data_ptr = &market_data->kline_data_1w;
                                        break;
                                    case 5: // 1月
                                        kline_data_ptr = &market_data->kline_data_1mo;
                                        break;
                                    case 12: // 3月
                                        kline_data_ptr = &market_data->kline_data_3mo;
                                        break;
                                    default:
                                        ESP_LOGW(TAG, "Unsupported kline type: %d", kline_type);
                                        break;
                                }
                                
                                // 使用已有的K线数据
                                if (kline_data_ptr) {
                                    existing_data = *kline_data_ptr;
                                }
                            }
                        }
                    }
                    
                    callback(existing_data);
                }
                return;
            }
            
            // 将当前currency_id和kline_type组合添加到正在进行的请求集合中
            pending_kline_requests_.insert(key);
        }
        
        // 创建一个新的任务来处理HTTP请求
        auto task_data = new KLineTaskData{currency_id, kline_type, limit, callback, proxy_config_, this};
        
        // 创建任务处理K线数据获取
        xTaskCreatePinnedToCore(
            KLineDataTask, 
            "kline_task", 
            8192, 
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
        Impl* impl;  // 添加Impl指针
    };
    
    static void KLineDataTask(void* pvParameters) {
        KLineTaskData* task_data = static_cast<KLineTaskData*>(pvParameters);
        
        std::vector<KLineData> kline_data;
        std::string url = "https://www.528btc.com/e/extend/api/index.php";
        
        // 根据货币ID设置符号
        std::string symbol, slug;
        switch (task_data->currency_id) {
            case 1: symbol = "BTC"; slug = "bitcoin"; break;
            case 2: symbol = "ETH"; slug = "ethereum"; break;
            case 17: symbol = "LTC"; slug = "litecoin"; break;
            case 4: symbol = "BNB"; slug = "binance-coin";break;
            case 5: symbol = "XRP"; slug = "ripple"; break;
            case 6: symbol = "ADA"; slug = "cardano"; break;
            case 7: symbol = "SOL"; slug = "solana"; break;
            case 8: symbol = "DOGE"; slug = "dogecoin"; break;
            case 14: symbol = "TRX"; slug = "tron"; break;
            case 23: symbol = "XLM"; slug = "stellar"; break;
            default: symbol = "UNK"; slug = "unknow"; break;
        }
        
        ESP_LOGI(TAG, "Fetching K-line data from: %s ", url.c_str());
        // 使用NetworkInterface中的HTTP客户端
        auto network = Board::GetInstance().GetNetwork();
        ESP_LOGI(TAG, "Network interface: %p", network);
        if (!network) {
            ESP_LOGE(TAG, "Network interface is not available");
            if (task_data->callback) {
                task_data->callback(kline_data);
            }
            
            // 从正在进行的请求集合中移除currency_id
            {
                std::lock_guard<std::mutex> lock(kline_requests_mutex_);
                pending_kline_requests_.erase({task_data->currency_id, task_data->kline_type});
                ESP_LOGI(TAG, "network faild, removed currency_id %d kline_type %d from pending requests", task_data->currency_id, task_data->kline_type);
            }
            
            delete task_data;
            vTaskDelete(nullptr);
            return;
        }
        
        // 创建通用HTTP客户端，支持代理设置
        std::unique_ptr<UniversalHttpClient> client = std::make_unique<UniversalHttpClient>(network);
        ESP_LOGI(TAG, "Created UniversalHttpClient: %p", client.get());
        if (!client) {
            ESP_LOGE(TAG, "Failed to create HTTP client");
            if (task_data->callback) {
                task_data->callback(kline_data);
            }
            
            // 从正在进行的请求集合中移除currency_id
            {
                std::lock_guard<std::mutex> lock(kline_requests_mutex_);
                pending_kline_requests_.erase({task_data->currency_id, task_data->kline_type});
                ESP_LOGI(TAG, "client faild, removed currency_id %d kline_type %d from pending requests", task_data->currency_id, task_data->kline_type);
            }
            
            delete task_data;
            vTaskDelete(nullptr);
            return;
        }
        
        // 设置代理（如果配置了代理）
        if (task_data->proxy_config.IsValid()) {
            ESP_LOGI(TAG, "Setting proxy: %s:%d", task_data->proxy_config.host.c_str(), task_data->proxy_config.port);
            client->SetProxy(task_data->proxy_config);
        } else {
            ESP_LOGI(TAG, "No proxy configured for K-line request");
        }
        
        // 设置请求头
        ESP_LOGV(TAG, "Setting request headers");
        client->SetHeader("Accept", "application/json, text/javascript, */*; q=0.01");
        client->SetHeader("Accept-Language", "zh-CN,zh;q=0.9");
        client->SetHeader("Cache-Control", "no-cache");
        client->SetHeader("Host", "www.528btc.com");
        client->SetHeader("Origin", "https://www.528btc.com");
        client->SetHeader("Referer", "https://www.528btc.com/coin/3008.html");
        client->SetHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/138.0.0.0 Safari/537.36");
        
        // 设置Params
        client->SetParam("m", "kline");
        client->SetParam("c", "coin");
        char buffer[32] = {0};
        itoa(task_data->currency_id, buffer, 10);
        client->SetParam("id", buffer);
        client->SetParam("symbol", symbol);
        client->SetParam("slug", slug);
        client->SetParam("sortByDate", "true");
        client->SetParam("sortByDateRule", "false");
        itoa(task_data->kline_type, buffer, 10);
        client->SetParam("type", buffer);
        itoa(task_data->limit, buffer, 10);
        client->SetParam("limit", buffer);
        client->SetParam("start", "");

        // 设置Cookie
        /*
        if (!client->SetCookie("__vtins__3ExGyQaAoNSqsSUY", "%7B%22sid%22%3A%20%22723e1fb3-b2aa-5172-b01f-fffa645921e9%22%2C%20%22vd%22%3A%203%2C%20%22stt%22%3A%208434%2C%20%22dr%22%3A%204345%2C%20%22expires%22%3A%201755104204431%2C%20%22ct%22%3A%201755102404431%7D")) {
            ESP_LOGE(TAG, "Failed to set cookie");
        }*/
        // 设置Cookie
        /*if (!client->SetCookie("__vtins__3ExGyQaAoNSqsSUY", "{\"sid\": \"723e1fb3-b2aa-5172-b01f-fffa645921e9\", \"vd\": 3, \"stt\": 8434, \"dr\": 4345, \"expires\": 1755104204431, \"ct\": 1755102404431}")) {
            ESP_LOGE(TAG, "Failed to set cookie");
        }
        */
        
        // 执行请求
        ESP_LOGI(TAG, "Opening HTTP connection");
        if (!client->Open("POST", url)) {
            ESP_LOGE(TAG, "Failed to open HTTP connection");
            if (task_data->callback) {
                task_data->callback(kline_data);
            }
            
            // 从正在进行的请求集合中移除currency_id
            {
                std::lock_guard<std::mutex> lock(kline_requests_mutex_);
                pending_kline_requests_.erase({task_data->currency_id, task_data->kline_type});
                ESP_LOGI(TAG, "Open url:%s failed, removed currency_id %d kline_type %d from pending requests", url.c_str(), task_data->currency_id, task_data->kline_type);
            }
            
            delete task_data;
            vTaskDelete(nullptr);
            return;
        }
        
        ESP_LOGI(TAG, "Getting HTTP status code");
        int status_code = client->GetStatusCode();
        ESP_LOGI(TAG, "HTTP status code: %d", status_code);
        if (status_code == 200) {
            // 获取响应数据
            ESP_LOGI(TAG, "Reading HTTP response data");
            std::string response = client->ReadAll();
            ESP_LOGI(TAG, "Response data size: %d", (int)response.size());
            if (!response.empty()) {
                ESP_LOGI(TAG, "Response: %s", response.c_str());
                
                // 解析JSON数据
                cJSON* root = cJSON_Parse(response.c_str());
                if (root) {
                    ESP_LOGI(TAG, "Successfully parsed JSON response");
                    // 解析K线数据数组
                    if (cJSON_IsArray(root)) {
                        int array_size = cJSON_GetArraySize(root);
                        ESP_LOGI(TAG, "K-line data array size: %d", array_size);
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
                        ESP_LOGI(TAG, "Parsed %d K-line data points", (int)kline_data.size());
                    }
                    cJSON_Delete(root);
                } else {
                    ESP_LOGE(TAG, "Failed to parse JSON response");
                }
            } else {
                ESP_LOGW(TAG, "Empty response received");
            }
        } else {
            ESP_LOGE(TAG, "HTTP request failed with status code: %d", status_code);
            
            // 尝试读取错误响应体
            std::string error_response = client->ReadAll();
            if (!error_response.empty()) {
                ESP_LOGE(TAG, "Error response body: %s", error_response.c_str());
            }
        }
        
        ESP_LOGI(TAG, "Closing HTTP connection");
        client->Close();
        
        // 调用回调函数
        ESP_LOGI(TAG, "Calling callback with %d K-line data points", (int)kline_data.size());
        if (task_data->callback) {
            task_data->callback(kline_data);
        }
        
        // 更新对应Connection的K线数据，以便UI查询时能获取到正确的数据
        if (!kline_data.empty() && task_data->impl) {
            // 获取对应的Connection
            auto connection_it = task_data->impl->connections_.find(task_data->currency_id);
            if (connection_it != task_data->impl->connections_.end()) {
                auto connection = connection_it->second;
                if (connection) {
                    auto market_data = connection->GetMarketData();
                    if (market_data) {
                        // 创建锁确保线程安全
                        std::lock_guard<std::mutex> lock(kline_requests_mutex_);
                                                        
                        // 根据kline_type更新对应的K线数据
                        switch (task_data->kline_type) {
                            case 13: // 1分钟
                                // 清除旧数据并复制新数据
                                market_data->kline_data_1m.clear();
                                market_data->kline_data_1m = kline_data;
                                ESP_LOGI(TAG, "Stored %d 1m K-line points for currency %d", 
                                        (int)kline_data.size(), task_data->currency_id);
                                break;
                            case 14: // 5分钟
                                market_data->kline_data_5m.clear();
                                market_data->kline_data_5m = kline_data;
                                ESP_LOGI(TAG, "Stored %d 5m K-line points for currency %d", 
                                        (int)kline_data.size(), task_data->currency_id);
                                break;
                            case 1: // 15分钟
                                market_data->kline_data_15m.clear();
                                market_data->kline_data_15m = kline_data;
                                ESP_LOGI(TAG, "Stored %d 15m K-line points for currency %d", 
                                        (int)kline_data.size(), task_data->currency_id);
                                break;
                            case 2: // 1小时
                                market_data->kline_data_1h.clear();
                                market_data->kline_data_1h = kline_data;
                                ESP_LOGI(TAG, "Stored %d 1h K-line points for currency %d", 
                                        (int)kline_data.size(), task_data->currency_id);
                                break;
                            case 10: // 2小时
                                market_data->kline_data_2h.clear();
                                market_data->kline_data_2h = kline_data;
                                ESP_LOGI(TAG, "Stored %d 2h K-line points for currency %d", 
                                        (int)kline_data.size(), task_data->currency_id);
                                break;
                            case 11: // 4小时
                                market_data->kline_data_4h.clear();
                                market_data->kline_data_4h = kline_data;
                                ESP_LOGI(TAG, "Stored %d 4h K-line points for currency %d", 
                                        (int)kline_data.size(), task_data->currency_id);
                                break;
                            case 3: // 1天
                                market_data->kline_data_1d.clear();
                                market_data->kline_data_1d = kline_data;
                                ESP_LOGI(TAG, "Stored %d 1d K-line points for currency %d", 
                                        (int)kline_data.size(), task_data->currency_id);
                                break;
                            case 4: // 1周
                                market_data->kline_data_1w.clear();
                                market_data->kline_data_1w = kline_data;
                                ESP_LOGI(TAG, "Stored %d 1w K-line points for currency %d", 
                                        (int)kline_data.size(), task_data->currency_id);
                                break;
                            case 5: // 1月
                                market_data->kline_data_1mo.clear();
                                market_data->kline_data_1mo = kline_data;
                                ESP_LOGI(TAG, "Stored %d 1mo K-line points for currency %d", 
                                        (int)kline_data.size(), task_data->currency_id);
                                break;
                            case 12: // 3月
                                market_data->kline_data_3mo.clear();
                                market_data->kline_data_3mo = kline_data;
                                ESP_LOGI(TAG, "Stored %d 3mo K-line points for currency %d", 
                                        (int)kline_data.size(), task_data->currency_id);
                                break;
                            default:
                                ESP_LOGW(TAG, "Unsupported kline type: %d", task_data->kline_type);
                                break;
                        }
                                                        
                        // 验证数据完整性
                        ESP_LOGI(TAG, "Verified storage: currency %d type %d has %d points", 
                                task_data->currency_id, task_data->kline_type, 
                                (int)kline_data.size());
                    } else {
                        ESP_LOGE(TAG, "Failed to get market data for currency %d", 
                                task_data->currency_id);
                    }
                }
            }
        }
        
        // 从正在进行的请求集合中移除currency_id
        {
            std::lock_guard<std::mutex> lock(kline_requests_mutex_);
            pending_kline_requests_.erase({task_data->currency_id, task_data->kline_type});
            ESP_LOGI(TAG, "Getkline finish, removed currency_id %d kline_type %d from pending requests", task_data->currency_id, task_data->kline_type);
        }        
        // 清理任务数据
        delete task_data;
        
        // 删除任务
        ESP_LOGI(TAG, "KLineDataTask completed, deleting task");
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

std::string CoinMarketData::toString() const {
    std::ostringstream oss;
    
    oss << "CoinMarketData{"
        << "timestamp=" << timestamp
        << ",open=" << std::fixed << std::setprecision(6) << open
        << ",high=" << std::fixed << std::setprecision(6) << high
        << ",low=" << std::fixed << std::setprecision(6) << low
        << ",close=" << std::fixed << std::setprecision(6) << close
        << ",turnover=" << std::fixed << std::setprecision(6) << turnover
        << ",change=" << std::fixed << std::setprecision(6) << change
        << ",change_24h=" << std::fixed << std::setprecision(6) << change_24h
        << ",change_7d=" << std::fixed << std::setprecision(6) << change_7d
        << ",change_30d=" << std::fixed << std::setprecision(6) << change_30d
        << ",change_1h=" << std::fixed << std::setprecision(6) << change_1h
        << ",change_year=" << std::fixed << std::setprecision(6) << change_year
        << ",change_now=" << std::fixed << std::setprecision(6) << change_now
        << ",currency_id=" << currency_id
        << ",circulation_market=" << std::fixed << std::setprecision(6) << circulation_market
        << ",kline_data_1m_size=" << kline_data_1m.size()
        << ",kline_data_5m_size=" << kline_data_5m.size()
        << ",kline_data_15m_size=" << kline_data_15m.size()
        << ",kline_data_1h_size=" << kline_data_1h.size()
        << ",kline_data_2h_size=" << kline_data_2h.size()
        << ",kline_data_4h_size=" << kline_data_4h.size()
        << ",kline_data_1d_size=" << kline_data_1d.size()
        << ",kline_data_1w_size=" << kline_data_1w.size()
        << ",kline_data_1mo_size=" << kline_data_1mo.size()
        << ",kline_data_3mo_size=" << kline_data_3mo.size()
        << "}";
    
    return oss.str();
}
