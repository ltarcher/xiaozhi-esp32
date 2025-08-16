#ifndef BIJIE_COINS_H
#define BIJIE_COINS_H

#include "universal_websocket.h"
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <memory>

#define MAX_COIN_NAME_LEN 16
#define MAX_KLINE_FREQUENCIES_LEN 10

// K线数据结构
struct KLineData {
    long long timestamp;  // 时间戳
    float open;           // 开盘价
    float high;           // 最高价
    float low;            // 最低价
    float close;          // 收盘价
    float volume;         // 成交量
};

// 虚拟币行情数据结构
struct CoinMarketData {
    // 实时行情数据
    long long timestamp;     // 时间戳
    float open;              // 开盘价
    float high;              // 最高价
    float low;               // 最低价
    float close;             // 收盘价/当前价
    float turnover;          // 成交量
    float change;            // 变化
    float change_24h;        // 24小时变化
    float change_7d;         // 7天变化
    float change_30d;        // 30天变化
    float change_1h;         // 1小时变化
    float change_year;       // 年变化
    float change_now;        // 当前变化
    int currency_id;         // 货币ID
    double circulation_market; // 流通市值
    
    // K线数据（历史数据）
    std::vector<KLineData> kline_data_1m;  // 1分钟K线
    std::vector<KLineData> kline_data_5m;  // 5分钟K线
    std::vector<KLineData> kline_data_15m; // 15分钟K线
    std::vector<KLineData> kline_data_1h;  // 1小时K线
    std::vector<KLineData> kline_data_2h;  // 2小时K线
    std::vector<KLineData> kline_data_4h;  // 4小时K线
    std::vector<KLineData> kline_data_1d;  // 1天K线
    std::vector<KLineData> kline_data_1w;  // 1周K线
    std::vector<KLineData> kline_data_1mo; // 1月K线
    std::vector<KLineData> kline_data_3mo; // 3月K线
    
    // toString函数，将所有变量转为一个字符串返回
    std::string toString() const;
};

// 币种信息结构
struct CoinInfo {
    int id;
    std::string symbol;
    std::string name;
    float price;
    float change_24h;
};

// 代理配置结构
struct ProxyConfig {
    std::string host;
    int port;
    std::string username;
    std::string password;
    
    bool IsValid() const {
        return !host.empty() && port > 0;
    }
};

// 回调函数类型定义
using OnMarketDataCallback = std::function<void(const CoinMarketData&)>;
using OnCoinListCallback = std::function<void(const std::vector<CoinInfo>&)>;
using OnKLineDataCallback = std::function<void(const std::vector<KLineData>&)>;

class BiJieCoins {
public:
    BiJieCoins();
    ~BiJieCoins();
    
    /**
     * @brief 设置代理配置
     * @param proxy 代理配置
     */
    void SetProxy(const ProxyConfig& proxy);
    
    /**
     * @brief 获取当前代理配置
     * @return 当前代理配置
     */
    ProxyConfig GetProxy() const;
    
    /**
     * @brief 连接到币界WebSocket服务器
     * @param currency_id 货币ID
     * @return true表示连接请求已发送，false表示连接失败
     */
    bool Connect(int currency_id);
    
    /**
     * @brief 断开指定货币ID的WebSocket连接
     * @param currency_id 货币ID
     */
    void Disconnect(int currency_id);
    
    /**
     * @brief 断开所有WebSocket连接
     */
    void DisconnectAll();
    
    /**
     * @brief 检查指定货币ID的WebSocket是否已连接
     * @param currency_id 货币ID
     * @return true表示已连接，false表示未连接
     */
    bool IsConnected(int currency_id) const;
    
    /**
     * @brief 获取指定货币的实时行情数据
     * @param currency_id 货币ID
     * @return CoinMarketData结构的智能指针，如果未找到则返回nullptr
     */
    std::shared_ptr<CoinMarketData> GetMarketData(int currency_id) const;
    
    /**
     * @brief 获取货币列表
     * @return 货币信息列表
     */
    std::vector<CoinInfo> GetCoinList() const;
    
    /**
     * @brief 设置行情数据回调函数
     * @param callback 回调函数
     */
    void SetMarketDataCallback(OnMarketDataCallback callback);
    
    /**
     * @brief 设置货币列表回调函数
     * @param callback 回调函数
     */
    void SetCoinListCallback(OnCoinListCallback callback);
    
    /**
     * @brief 获取支持的货币ID列表
     * @return 货币ID列表
     */
    std::vector<int> GetSupportedCurrencyIds() const;
    
    /**
     * @brief 获取指定货币的历史K线数据
     * @param currency_id 货币ID
     * @param kline_type K线类型, 默认为2（1小时K线），
     *                  可选值：13（1分钟），
     *                          14（5分钟），
     *                          1（15分钟），
     *                          2（1小时），
     *                          10（2小时），
     *                          11（4小时），
     *                          3（1天），
     *                          4（1周）， 
     *                          5（1月），
     *                          12（三个月）
     * @param limit 数据条数限制
     * @param callback 回调函数
     */
    void GetKLineData(int currency_id, int kline_type, int limit, OnKLineDataCallback callback);

    /**
     * @brief 获取K线时间频率列表
     * @return K线时间频率列表
     */
    const char** GetKLineTimeFrequencies() {
        static const char* klinefreq_[] = {
            "1m", "5m", "15m", "1h", "2h", "4h", "1d", "1w", "1mo", "3mo"
        };
        return klinefreq_;
    };
private:
    class Impl;  // 前向声明实现类
    std::unique_ptr<Impl> impl_;
};

#endif // BIJIE_COINS_H