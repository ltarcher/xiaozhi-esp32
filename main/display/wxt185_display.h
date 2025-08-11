#ifndef _WXT185_DISPLAY_H_
#define _WXT185_DISPLAY_H_

#include "lcd_display.h"
#include <lvgl.h>
#include <vector>
#include <string>

// 虚拟币数据结构
struct CryptocurrencyData {
    std::string symbol;
    std::string name;
    float price;
    float change_24h;
    // K线数据
    std::vector<std::pair<float, float>> kline_data_1m;
    std::vector<std::pair<float, float>> kline_data_5m;
    std::vector<std::pair<float, float>> kline_data_15m;
    std::vector<std::pair<float, float>> kline_data_1h;
    std::vector<std::pair<float, float>> kline_data_4h;
    std::vector<std::pair<float, float>> kline_data_1d;
    std::vector<std::pair<float, float>> kline_data_1w;
    std::vector<std::pair<float, float>> kline_data_1mo;
    std::vector<std::pair<float, float>> kline_data_3mo;
};

// 主题样式枚举
enum class ThemeStyle {
    LIGHT,
    DARK,
    METAL,
    TECHNOLOGY,
    COSMIC
};

// WXT185主题颜色定义
struct WXT185ThemeColors {
    lv_color_t background;
    lv_color_t text;
    lv_color_t chat_background;
    lv_color_t user_bubble;
    lv_color_t assistant_bubble;
    lv_color_t system_bubble;
    lv_color_t system_text;
    lv_color_t border;
    lv_color_t low_battery;
    lv_color_t header;
    lv_color_t selector;
};

class WXT185Display : public SpiLcdDisplay {
protected:
    lv_obj_t* main_screen_ = nullptr;
    lv_obj_t* page_view_ = nullptr;
    
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
    lv_obj_t* crypto_chart_ = nullptr;
    lv_obj_t* crypto_list_ = nullptr;
    lv_obj_t* crypto_time_selector_ = nullptr;
    
    // 设置页面组件
    lv_obj_t* settings_header_ = nullptr;
    lv_obj_t* settings_theme_selector_ = nullptr;
    lv_obj_t* settings_crypto_selector_ = nullptr;
    lv_obj_t* settings_timeframe_selector_ = nullptr;
    
    // 数据
    std::vector<CryptocurrencyData> crypto_data_;
    std::vector<std::string> selected_cryptos_;
    std::string current_timeframe_;
    ThemeStyle current_theme_style_;
    
    // 主题颜色
    WXT185ThemeColors current_wxt185_theme_;
    
    // 触摸滑动相关
    int current_page_index_ = 0;
    lv_point_t touch_start_point_ = {0, 0};
    bool is_touching_ = false;
    
    void SetupUI() override;
    
    // 页面创建函数
    void CreateChatPage();
    void CreateCryptoPage();
    void CreateSettingsPage();
    
    // 主题应用函数
    void ApplyTheme();
    void ApplyChatPageTheme();
    void ApplyCryptoPageTheme();
    void ApplySettingsPageTheme();
    
    // 虚拟币数据处理函数
    void UpdateCryptoData();
    void DrawKLineChart();
    
    // 事件处理函数
    static void PageEventHandler(lv_event_t* e);
    static void CryptoSelectorEventHandler(lv_event_t* e);
    static void ThemeSelectorEventHandler(lv_event_t* e);
    static void TimeframeSelectorEventHandler(lv_event_t* e);
    
    // 触摸事件处理
    static void TouchEventHandler(lv_event_t* e);
    void HandleTouchStart(lv_point_t point);
    void HandleTouchEnd(lv_point_t point);

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
};

#endif // _WXT185_DISPLAY_H_