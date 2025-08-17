#ifndef _WXT185_DISPLAY_H_
#define _WXT185_DISPLAY_H_

#include "lcd_display.h"
#include "protocols/bijie_coins.h"
#include <lvgl.h>
#include <vector>
#include <string>
#include <memory>
#include "http_client.h"  // 添加HTTP客户端头文件

// 屏保虚拟币数据结构
struct CryptocurrencyData {
    std::string symbol;
    std::string name;
    float price;
    float change_24h;
    int currency_id; // 货币ID
    
    // K线数据（历史数据）
    std::vector<KLineData> kline_data_1m;  // 1分钟K线
    std::vector<KLineData> kline_data_5m;  // 5分钟K线
    std::vector<KLineData> kline_data_15m; // 15分钟K线
    std::vector<KLineData> kline_data_1h;  // 1小时K线
    std::vector<KLineData> kline_data_4h;  // 4小时K线
    std::vector<KLineData> kline_data_1d;  // 1天K线
    std::vector<KLineData> kline_data_1w;  // 1周K线
    std::vector<KLineData> kline_data_1mo; // 1月K线
    std::vector<KLineData> kline_data_3mo; // 3月K线
};

// WXT185主题颜色定义
struct WXT185ThemeColors {
    lv_color_t background;
    lv_color_t text;
    // 外圆环颜色
    lv_color_t outer_ring_background;
    // 内圆环颜色
    lv_color_t inner_ring_background;
    // 聊天页面样式
    lv_color_t chat_background;
    lv_color_t user_bubble;
    lv_color_t assistant_bubble;
    lv_color_t system_bubble;
    lv_color_t system_text;
    lv_color_t border;
    lv_color_t low_battery;
    lv_color_t header;
    lv_color_t selector;
    // 虚拟币页面样式
    lv_color_t crypto_background;
    lv_color_t crypto_text;
    lv_color_t crypto_sub_text;
    lv_color_t crypto_up_color;
    lv_color_t crypto_down_color;
    lv_color_t crypto_border_color;
    lv_color_t crypto_progress_bg_color;
    // 设置页面样式
    lv_color_t settings_screensaver_switch;
};

// 定义页面
enum Page {
    PAGE_CHAT,
    PAGE_CRYPTO,
    PAGE_SETTINGS,
    MAX_PAGE_INDEX
};

// 前向声明用于友元函数
class WXT185Display;
static void update_crypto_data_async(void* user_data);
static void process_kline_data_async(void* user_data);

class WXT185Display : public LcdDisplay {
    // 声明友元函数
    friend void update_crypto_data_async(void* user_data);
    friend void process_kline_data_async(void* user_data);
    
protected:
    lv_obj_t* main_screen_ = nullptr;
    // 页面通用组件
    lv_obj_t* common_outer_ring_ = nullptr;
    lv_obj_t* common_inner_ring_ = nullptr;
    
    // 三个主要页面
    lv_obj_t* chat_page_ = nullptr;
    lv_obj_t* crypto_page_ = nullptr;
    lv_obj_t* settings_page_ = nullptr;
    
    // 聊天页面组件
    lv_obj_t* chat_status_bar_ = nullptr;
    lv_obj_t* chat_content_ = nullptr;
    lv_obj_t* chat_input_area_ = nullptr;
    
    // 虚拟币页面组件
    lv_obj_t* crypto_header_ = nullptr;
    lv_obj_t* crypto_roller_ = nullptr;
    lv_obj_t* crypto_kline_btn_container_ = nullptr;
    lv_obj_t* crypto_content_ = nullptr;
    lv_obj_t* crypto_price_label_ = nullptr;
    lv_obj_t* crypto_change_label_ = nullptr;
    lv_obj_t* crypto_chart_ = nullptr;
    
    lv_obj_t* kline_frequency_buttons_[10];  // K线频率按钮数组
    int selected_kline_frequency_ = 3; // 默认选择1小时K线 (对应索引3)
    
    // 设置页面组件
    lv_obj_t* settings_title_ = nullptr;
    lv_obj_t* settings_theme_label_ = nullptr;
    lv_obj_t* settings_theme_roller_ = nullptr;
    lv_obj_t* settings_default_crypto_label_ = nullptr;
    lv_obj_t* settings_default_crypto_roller_ = nullptr;
    lv_obj_t* settings_kline_time_label_ = nullptr;
    lv_obj_t* settings_kline_time_roller_ = nullptr;
    lv_obj_t* settings_screensaver_label_ = nullptr;
    lv_obj_t* settings_screensaver_switch_ = nullptr;
    lv_obj_t* settings_save_button_ = nullptr;  // 添加保存按钮
    lv_obj_t* settings_save_label_ = nullptr;
    
    // 屏幕保护相关组件
    lv_obj_t* screensaver_page_ = nullptr;
    lv_obj_t* screensaver_outer_ring_ = nullptr;
    lv_obj_t* screensaver_progress_ring_ = nullptr;
    lv_obj_t* screensaver_crypto_name_ = nullptr;
    lv_obj_t* screensaver_crypto_fullname_ = nullptr;
    lv_obj_t* screensaver_crypto_price_ = nullptr;
    lv_obj_t* screensaver_crypto_change_ = nullptr;
    lv_obj_t* screensaver_time_ = nullptr;
    
    // 数据
    // 当前选择显示的虚拟币
    CryptocurrencyData current_crypto_data_;
    std::vector<CryptocurrencyData> crypto_data_;
    std::string current_timeframe_;
    
    // 主题颜色
    WXT185ThemeColors current_wxt185_theme_;
    
    // 触摸滑动相关
    lv_point_t touch_start_point_ = {0, 0};
    bool is_touching_ = false;
    
public:
    static uint32_t current_page_index_;
    // 屏幕保护相关
    bool screensaver_active_ = false;
    int64_t last_activity_time_ = 0;
    esp_timer_handle_t screensaver_timer_ = nullptr;
    CryptocurrencyData screensaver_crypto_;
    std::vector<std::pair<float, float>> screensaver_kline_data_; // 屏保K线数据

    // 虚拟币行情更新定时器
    esp_timer_handle_t crypto_update_timer_ = nullptr;

    // 屏保时间更新定时器
    esp_timer_handle_t screensaver_time_update_timer_ = nullptr;

protected:
    // 控制虚拟币行情获取的变量
    bool enable_realtime_crypto_data_ = true;   // 是否启用实时行情获取，默认启用
    bool enable_kline_crypto_data_ = true;     // 是否启用历史K线行情获取，默认启用

    // 代理配置
    ProxyConfig proxy_config_;
    
    // 币界虚拟币行情数据支持
    std::unique_ptr<BiJieCoins> bijie_coins_;
    bool bijie_coins_connected_ = false;
    
    void SetupUI();
    
    // 通用页面组件
    void CreateCommonComponents();
    // 页面创建函数
    void CreateChatPage();
    void CreateCryptoPage();
    void CreateSettingsPage();
    void CreateScreensaverPage(); // 新增屏幕保护页面创建函数
    
    // 主题应用函数
    void ApplyTheme();
    void ApplyChatPageTheme();
    void ApplyCryptoPageTheme();
    void ApplySettingsPageTheme();
    void ApplyScreensaverTheme(); // 新增屏幕保护主题应用函数
    
    // 虚拟币数据处理函数
    void UpdateCryptoData();
    void DrawKLineChart();
    uint32_t GetKLineTypeByIndex(uint8_t index);
    
    // 事件处理函数
    static void PageEventHandler(lv_event_t* e);
    static void CryptoSelectorEventHandler(lv_event_t* e);
    static void ThemeSelectorEventHandler(lv_event_t* e);
    static void TimeframeSelectorEventHandler(lv_event_t* e);
    static void KLineFrequencyButtonEventHandler(lv_event_t* e); // K线频率按钮事件处理
    
    // 触摸事件处理
    static void TouchEventHandler(lv_event_t* e);
    void HandleTouchStart(lv_point_t point);
    void HandleTouchEnd(lv_point_t point);
    
public:
    void SaveSettings(); // 保存设置方法
    // 屏幕保护相关函数
    static void ScreensaverTimerCallback(void* arg);
    void StartScreensaverTimer();
    void StopScreensaverTimer();
    void EnterScreensaver();
    void ExitScreensaver();
    void UpdateScreensaverContent();
    
    // 虚拟币行情更新相关函数
    static void CryptoUpdateTimerCallback(void* arg);
    void StartCryptoUpdateTimer();
    void StopCryptoUpdateTimer();
    
    // 屏保时间更新相关函数
    static void ScreensaverTimeUpdateTimerCallback(void* arg);
    void StartScreensaverTimeUpdateTimer();
    void StopScreensaverTimeUpdateTimer();
    void UpdateScreensaverTime();

public:
    WXT185Display(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                  int width, int height, int offset_x, int offset_y,
                  bool mirror_x, bool mirror_y, bool swap_xy,
                  DisplayFonts fonts);
    
    ~WXT185Display();
    
    // 重写基础显示函数
    void SetEmotion(const char* emotion) override;
    void SetIcon(const char* icon) override;
    void SetPreviewImage(const lv_img_dsc_t* img_dsc) override;
    void SetChatMessage(const char* role, const char* content) override;
    void SetTheme(const std::string& theme_name) override;
    
    // 虚拟币相关函数
    void AddCryptocurrency(const CryptocurrencyData& crypto);
    void RemoveCryptocurrency(const std::string& symbol);
    void UpdateCryptocurrencyPrice(const std::string& symbol, float price, float change);
    
    // 页面切换函数
    void SwitchToPage(int page_index);
    
    // 屏幕保护公共接口
    void OnActivity(); // 用户活动时调用
    void OnConversationStart(); // 对话开始时调用
    void OnConversationEnd(); // 对话结束时调用
    void OnIdle(); // 空闲状态时调用
    
    // 与Application集成的屏幕保护控制方法
    void OnDeviceStateChanged(int previous_state, int current_state); // 设备状态改变时调用
    
    /**
     * @brief 检查网络是否就绪
     * @param max_wait_time 最大等待时间(毫秒)，默认30秒
     * @return true表示网络就绪，false表示网络未就绪
     */
    bool WaitForNetworkReady(int max_wait_time = 30000);
        
    // 币界虚拟币行情数据相关函数
    void InitializeBiJieCoins(); // 初始化币界虚拟币行情数据
    void ConnectToBiJieCoins(); // 连接到币界虚拟币行情数据
    void UpdateCryptoDataFromBiJie(); // 从币界更新虚拟币数据
    void SwitchCrypto(int currency_id); // 切换当前显示的虚拟币
    void SetScreensaverCrypto(int currency_id); // 设置屏保显示的虚拟币
};

#endif // _WXT185_DISPLAY_H_