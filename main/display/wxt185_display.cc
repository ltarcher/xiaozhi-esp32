#include "wxt185_display.h"
#include <esp_log.h>
#include <algorithm>
#include <ctime>
#include "assets/lang_config.h"

#define TAG "WXT185Display"
#define SCREENSAVER_TIMEOUT_MS 10000 // 10秒超时进入屏保

// 颜色定义 - LIGHT主题
#define LIGHT_BACKGROUND_COLOR       lv_color_white()
#define LIGHT_TEXT_COLOR             lv_color_black()
#define LIGHT_CHAT_BACKGROUND_COLOR  lv_color_hex(0xE0E0E0)
#define LIGHT_USER_BUBBLE_COLOR      lv_color_hex(0x1A6C37)
#define LIGHT_ASSISTANT_BUBBLE_COLOR lv_color_white()
#define LIGHT_SYSTEM_BUBBLE_COLOR    lv_color_hex(0xE0E0E0)
#define LIGHT_SYSTEM_TEXT_COLOR      lv_color_hex(0x666666)
#define LIGHT_BORDER_COLOR           lv_color_hex(0xE0E0E0)
#define LIGHT_LOW_BATTERY_COLOR      lv_color_black()
#define LIGHT_HEADER_COLOR           lv_color_hex(0xDDDDDD)
#define LIGHT_SELECTOR_COLOR         lv_color_hex(0xF0F0F0)

// 颜色定义 - DARK主题
#define DARK_BACKGROUND_COLOR        lv_color_hex(0x121212)
#define DARK_TEXT_COLOR              lv_color_white()
#define DARK_CHAT_BACKGROUND_COLOR   lv_color_hex(0x1E1E1E)
#define DARK_USER_BUBBLE_COLOR       lv_color_hex(0x1A6C37)
#define DARK_ASSISTANT_BUBBLE_COLOR  lv_color_hex(0x333333)
#define DARK_SYSTEM_BUBBLE_COLOR     lv_color_hex(0x2A2A2A)
#define DARK_SYSTEM_TEXT_COLOR       lv_color_hex(0xAAAAAA)
#define DARK_BORDER_COLOR            lv_color_hex(0x333333)
#define DARK_LOW_BATTERY_COLOR       lv_color_hex(0xFF0000)
#define DARK_HEADER_COLOR            lv_color_hex(0x252525)
#define DARK_SELECTOR_COLOR          lv_color_hex(0x303030)

// 颜色定义 - METAL主题
#define METAL_BACKGROUND_COLOR       lv_color_hex(0xC0C0C0)
#define METAL_TEXT_COLOR             lv_color_black()
#define METAL_CHAT_BACKGROUND_COLOR  lv_color_hex(0xA9A9A9)
#define METAL_USER_BUBBLE_COLOR      lv_color_hex(0x1A6C37)
#define METAL_ASSISTANT_BUBBLE_COLOR lv_color_hex(0xD3D3D3)
#define METAL_SYSTEM_BUBBLE_COLOR    lv_color_hex(0x808080)
#define METAL_SYSTEM_TEXT_COLOR      lv_color_hex(0x333333)
#define METAL_BORDER_COLOR           lv_color_hex(0x696969)
#define METAL_LOW_BATTERY_COLOR      lv_color_hex(0x8B0000)
#define METAL_HEADER_COLOR           lv_color_hex(0xB0B0B0)
#define METAL_SELECTOR_COLOR         lv_color_hex(0xCFCFCF)

// 颜色定义 - TECHNOLOGY主题
#define TECHNOLOGY_BACKGROUND_COLOR  lv_color_hex(0x0A0A20)
#define TECHNOLOGY_TEXT_COLOR        lv_color_hex(0x00FFFF)
#define TECHNOLOGY_CHAT_BACKGROUND_COLOR lv_color_hex(0x101030)
#define TECHNOLOGY_USER_BUBBLE_COLOR lv_color_hex(0x006400)
#define TECHNOLOGY_ASSISTANT_BUBBLE_COLOR lv_color_hex(0x151540)
#define TECHNOLOGY_SYSTEM_BUBBLE_COLOR lv_color_hex(0x202050)
#define TECHNOLOGY_SYSTEM_TEXT_COLOR lv_color_hex(0x00CED1)
#define TECHNOLOGY_BORDER_COLOR      lv_color_hex(0x000080)
#define TECHNOLOGY_LOW_BATTERY_COLOR lv_color_hex(0xFF4500)
#define TECHNOLOGY_HEADER_COLOR      lv_color_hex(0x151535)
#define TECHNOLOGY_SELECTOR_COLOR    lv_color_hex(0x202045)

// 颜色定义 - COSMIC主题
#define COSMIC_BACKGROUND_COLOR      lv_color_black()
#define COSMIC_TEXT_COLOR            lv_color_hex(0xFFFF00)
#define COSMIC_CHAT_BACKGROUND_COLOR lv_color_hex(0x100010)
#define COSMIC_USER_BUBBLE_COLOR     lv_color_hex(0x4B0082)
#define COSMIC_ASSISTANT_BUBBLE_COLOR lv_color_hex(0x200020)
#define COSMIC_SYSTEM_BUBBLE_COLOR   lv_color_hex(0x300030)
#define COSMIC_SYSTEM_TEXT_COLOR     lv_color_hex(0xDA70D6)
#define COSMIC_BORDER_COLOR          lv_color_hex(0x800080)
#define COSMIC_LOW_BATTERY_COLOR     lv_color_hex(0xFF0000)
#define COSMIC_HEADER_COLOR          lv_color_hex(0x150015)
#define COSMIC_SELECTOR_COLOR        lv_color_hex(0x250025)

// 主题定义
const WXT185ThemeColors LIGHT_THEME_WXT185 = {
    .background = LIGHT_BACKGROUND_COLOR,
    .text = LIGHT_TEXT_COLOR,
    .chat_background = LIGHT_CHAT_BACKGROUND_COLOR,
    .user_bubble = LIGHT_USER_BUBBLE_COLOR,
    .assistant_bubble = LIGHT_ASSISTANT_BUBBLE_COLOR,
    .system_bubble = LIGHT_SYSTEM_BUBBLE_COLOR,
    .system_text = LIGHT_SYSTEM_TEXT_COLOR,
    .border = LIGHT_BORDER_COLOR,
    .low_battery = LIGHT_LOW_BATTERY_COLOR,
    .header = LIGHT_HEADER_COLOR,
    .selector = LIGHT_SELECTOR_COLOR
};

const WXT185ThemeColors DARK_THEME_WXT185 = {
    .background = DARK_BACKGROUND_COLOR,
    .text = DARK_TEXT_COLOR,
    .chat_background = DARK_CHAT_BACKGROUND_COLOR,
    .user_bubble = DARK_USER_BUBBLE_COLOR,
    .assistant_bubble = DARK_ASSISTANT_BUBBLE_COLOR,
    .system_bubble = DARK_SYSTEM_BUBBLE_COLOR,
    .system_text = DARK_SYSTEM_TEXT_COLOR,
    .border = DARK_BORDER_COLOR,
    .low_battery = DARK_LOW_BATTERY_COLOR,
    .header = DARK_HEADER_COLOR,
    .selector = DARK_SELECTOR_COLOR
};

const WXT185ThemeColors METAL_THEME_WXT185 = {
    .background = METAL_BACKGROUND_COLOR,
    .text = METAL_TEXT_COLOR,
    .chat_background = METAL_CHAT_BACKGROUND_COLOR,
    .user_bubble = METAL_USER_BUBBLE_COLOR,
    .assistant_bubble = METAL_ASSISTANT_BUBBLE_COLOR,
    .system_bubble = METAL_SYSTEM_BUBBLE_COLOR,
    .system_text = METAL_SYSTEM_TEXT_COLOR,
    .border = METAL_BORDER_COLOR,
    .low_battery = METAL_LOW_BATTERY_COLOR,
    .header = METAL_HEADER_COLOR,
    .selector = METAL_SELECTOR_COLOR
};

const WXT185ThemeColors TECHNOLOGY_THEME_WXT185 = {
    .background = TECHNOLOGY_BACKGROUND_COLOR,
    .text = TECHNOLOGY_TEXT_COLOR,
    .chat_background = TECHNOLOGY_CHAT_BACKGROUND_COLOR,
    .user_bubble = TECHNOLOGY_USER_BUBBLE_COLOR,
    .assistant_bubble = TECHNOLOGY_ASSISTANT_BUBBLE_COLOR,
    .system_bubble = TECHNOLOGY_SYSTEM_BUBBLE_COLOR,
    .system_text = TECHNOLOGY_SYSTEM_TEXT_COLOR,
    .border = TECHNOLOGY_BORDER_COLOR,
    .low_battery = TECHNOLOGY_LOW_BATTERY_COLOR,
    .header = TECHNOLOGY_HEADER_COLOR,
    .selector = TECHNOLOGY_SELECTOR_COLOR
};

const WXT185ThemeColors COSMIC_THEME_WXT185 = {
    .background = COSMIC_BACKGROUND_COLOR,
    .text = COSMIC_TEXT_COLOR,
    .chat_background = COSMIC_CHAT_BACKGROUND_COLOR,
    .user_bubble = COSMIC_USER_BUBBLE_COLOR,
    .assistant_bubble = COSMIC_ASSISTANT_BUBBLE_COLOR,
    .system_bubble = COSMIC_SYSTEM_BUBBLE_COLOR,
    .system_text = COSMIC_SYSTEM_TEXT_COLOR,
    .border = COSMIC_BORDER_COLOR,
    .low_battery = COSMIC_LOW_BATTERY_COLOR,
    .header = COSMIC_HEADER_COLOR,
    .selector = COSMIC_SELECTOR_COLOR
};

WXT185Display::WXT185Display(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                           int width, int height, int offset_x, int offset_y,
                           bool mirror_x, bool mirror_y, bool swap_xy,
                           DisplayFonts fonts)
    : SpiLcdDisplay(panel_io, panel, width, height, offset_x, offset_y, 
                    mirror_x, mirror_y, swap_xy, fonts) {
    
    // 初始化默认设置
    current_timeframe_ = "1h";
    current_theme_style_ = ThemeStyle::TECHNOLOGY;
    current_wxt185_theme_ = TECHNOLOGY_THEME_WXT185;
    selected_cryptos_ = {"BTC", "ETH", "ADA"};
    
    // 初始化一些虚拟币数据
    CryptocurrencyData btc = {"BTC", "Bitcoin", 45000.0f, 2.5f};
    CryptocurrencyData eth = {"ETH", "Ethereum", 3000.0f, 1.2f};
    CryptocurrencyData ada = {"ADA", "Cardano", 1.2f, -0.8f};
    
    crypto_data_.push_back(btc);
    crypto_data_.push_back(eth);
    crypto_data_.push_back(ada);
    
    // 初始化最后活动时间为当前时间
    last_activity_time_ = esp_timer_get_time() / 1000; // 转换为毫秒
    
    // 创建屏保定时器
    esp_timer_create_args_t timer_args = {
        .callback = ScreensaverTimerCallback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "screensaver_timer",
        .skip_unhandled_events = false,
    };
    esp_timer_create(&timer_args, &screensaver_timer_);
}

WXT185Display::~WXT185Display() {
    // 删除屏保定时器
    if (screensaver_timer_) {
        esp_timer_stop(screensaver_timer_);
        esp_timer_delete(screensaver_timer_);
    }
}

void WXT185Display::SetupUI() {
    // 先调用父类的SetupUI来初始化基础界面元素
    SpiLcdDisplay::SetupUI();
    
    DisplayLockGuard lock(this);
    
    // 获取屏幕对象
    main_screen_ = lv_screen_active();
    
    // 创建页面视图容器（针对360*360圆形屏幕优化）
    page_view_ = lv_obj_create(main_screen_);
    lv_obj_set_size(page_view_, width_, height_);
    lv_obj_set_style_pad_all(page_view_, 0, 0);
    lv_obj_set_style_border_width(page_view_, 0, 0);
    lv_obj_set_style_bg_opa(page_view_, LV_OPA_TRANSP, 0);
    lv_obj_center(page_view_);
    
    // 添加触摸事件处理
    lv_obj_add_event_cb(page_view_, TouchEventHandler, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(page_view_, TouchEventHandler, LV_EVENT_RELEASED, this);
    
    // 创建四个页面
    CreateChatPage();
    CreateCryptoPage();
    CreateSettingsPage();
    CreateScreensaverPage(); // 创建屏保页面
    
    // 应用主题
    ApplyTheme();
    
    // 启动屏保定时器
    StartScreensaverTimer();
}

void WXT185Display::CreateChatPage() {
    chat_page_ = lv_obj_create(page_view_);
    lv_obj_set_size(chat_page_, width_, height_);
    lv_obj_set_style_pad_all(chat_page_, 0, 0);
    lv_obj_set_style_border_width(chat_page_, 0, 0);
    lv_obj_set_style_bg_opa(chat_page_, LV_OPA_TRANSP, 0);
    
    // 添加触摸事件处理
    lv_obj_add_event_cb(chat_page_, TouchEventHandler, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(chat_page_, TouchEventHandler, LV_EVENT_RELEASED, this);
    
    // 创建聊天状态栏（针对360*360屏幕优化高度）
    chat_status_bar_ = lv_obj_create(chat_page_);
    lv_obj_set_size(chat_status_bar_, width_, 35);
    lv_obj_set_style_radius(chat_status_bar_, 0, 0);
    lv_obj_set_style_pad_all(chat_status_bar_, 5, 0);
    lv_obj_align(chat_status_bar_, LV_ALIGN_TOP_MID, 0, 0);
    
    // 创建聊天内容区域（为圆形屏幕优化显示区域）
    chat_content_ = lv_obj_create(chat_page_);
    lv_obj_set_size(chat_content_, width_ - 20, height_ - 90);
    lv_obj_align_to(chat_content_, chat_status_bar_, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_obj_set_style_pad_all(chat_content_, 5, 0);
    lv_obj_set_style_border_width(chat_content_, 1, 0);
    lv_obj_set_scrollbar_mode(chat_content_, LV_SCROLLBAR_MODE_AUTO);
    
    // 创建输入区域
    chat_input_area_ = lv_obj_create(chat_page_);
    lv_obj_set_size(chat_input_area_, width_ - 20, 35);
    lv_obj_align(chat_input_area_, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_pad_all(chat_input_area_, 5, 0);
    
    // 在状态栏添加组件
    emotion_label_ = lv_label_create(chat_status_bar_);
    lv_label_set_text(emotion_label_, "AI");
    lv_obj_align(emotion_label_, LV_ALIGN_LEFT_MID, 0, 0);
    
    status_label_ = lv_label_create(chat_status_bar_);
    lv_label_set_text(status_label_, Lang::Strings::INITIALIZING);
    lv_obj_align(status_label_, LV_ALIGN_CENTER, 0, 0);
    
    battery_label_ = lv_label_create(chat_status_bar_);
    lv_label_set_text(battery_label_, "100%");
    lv_obj_align(battery_label_, LV_ALIGN_RIGHT_MID, 0, 0);
}

void WXT185Display::CreateCryptoPage() {
    crypto_page_ = lv_obj_create(page_view_);
    lv_obj_set_size(crypto_page_, width_, height_);
    lv_obj_set_style_pad_all(crypto_page_, 0, 0);
    lv_obj_set_style_border_width(crypto_page_, 0, 0);
    lv_obj_set_style_bg_opa(crypto_page_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_row(crypto_page_, 5, 0);
    lv_obj_set_flex_flow(crypto_page_, LV_FLEX_FLOW_COLUMN);
    
    // 添加触摸事件处理
    lv_obj_add_event_cb(crypto_page_, TouchEventHandler, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(crypto_page_, TouchEventHandler, LV_EVENT_RELEASED, this);
    
    // 创建头部区域
    crypto_header_ = lv_obj_create(crypto_page_);
    lv_obj_set_size(crypto_header_, width_, 35);
    lv_obj_set_style_pad_all(crypto_header_, 5, 0);
    lv_obj_set_style_border_width(crypto_header_, 0, 0);
    lv_obj_set_flex_grow(crypto_header_, 0);
    
    // 创建K线图表区域（为360*360屏幕优化大小）
    crypto_chart_ = lv_obj_create(crypto_page_);
    lv_obj_set_size(crypto_chart_, width_ - 20, 120);
    lv_obj_set_style_pad_all(crypto_chart_, 5, 0);
    lv_obj_set_style_border_width(crypto_chart_, 1, 0);
    lv_obj_set_flex_grow(crypto_chart_, 1);
    
    // 创建虚拟币列表区域
    crypto_list_ = lv_obj_create(crypto_page_);
    lv_obj_set_size(crypto_list_, width_ - 20, height_ - 210);
    lv_obj_set_style_pad_all(crypto_list_, 5, 0);
    lv_obj_set_style_border_width(crypto_list_, 1, 0);
    lv_obj_set_flex_grow(crypto_list_, 2);
    lv_obj_set_scrollbar_mode(crypto_list_, LV_SCROLLBAR_MODE_AUTO);
    
    // 创建时间选择器
    crypto_time_selector_ = lv_obj_create(crypto_page_);
    lv_obj_set_size(crypto_time_selector_, width_ - 20, 30);
    lv_obj_set_style_pad_all(crypto_time_selector_, 2, 0);
    lv_obj_set_style_border_width(crypto_time_selector_, 1, 0);
    lv_obj_set_flex_grow(crypto_time_selector_, 0);
    
    // 添加标题
    lv_obj_t* title = lv_label_create(crypto_header_);
    lv_label_set_text(title, "Crypto Market");
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 0, 0);
    
    // 添加刷新按钮
    lv_obj_t* refresh_btn = lv_btn_create(crypto_header_);
    lv_obj_set_size(refresh_btn, 60, 25);
    lv_obj_align(refresh_btn, LV_ALIGN_RIGHT_MID, 0, 0);
    
    lv_obj_t* refresh_label = lv_label_create(refresh_btn);
    lv_label_set_text(refresh_label, "Refresh");
    lv_obj_center(refresh_label);
}

void WXT185Display::CreateSettingsPage() {
    settings_page_ = lv_obj_create(page_view_);
    lv_obj_set_size(settings_page_, width_, height_);
    lv_obj_set_style_pad_all(settings_page_, 0, 0);
    lv_obj_set_style_border_width(settings_page_, 0, 0);
    lv_obj_set_style_bg_opa(settings_page_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_row(settings_page_, 5, 0);
    lv_obj_set_flex_flow(settings_page_, LV_FLEX_FLOW_COLUMN);
    
    // 添加触摸事件处理
    lv_obj_add_event_cb(settings_page_, TouchEventHandler, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(settings_page_, TouchEventHandler, LV_EVENT_RELEASED, this);
    
    // 创建设置页面头部
    settings_header_ = lv_obj_create(settings_page_);
    lv_obj_set_size(settings_header_, width_, 35);
    lv_obj_set_style_pad_all(settings_header_, 5, 0);
    lv_obj_set_style_border_width(settings_header_, 0, 0);
    lv_obj_set_flex_grow(settings_header_, 0);
    
    // 创建主题选择区域
    settings_theme_selector_ = lv_obj_create(settings_page_);
    lv_obj_set_size(settings_theme_selector_, width_ - 20, 70);
    lv_obj_set_style_pad_all(settings_theme_selector_, 5, 0);
    lv_obj_set_style_border_width(settings_theme_selector_, 1, 0);
    lv_obj_set_flex_grow(settings_theme_selector_, 0);
    
    // 创建虚拟币选择区域
    settings_crypto_selector_ = lv_obj_create(settings_page_);
    lv_obj_set_size(settings_crypto_selector_, width_ - 20, 110);
    lv_obj_set_style_pad_all(settings_crypto_selector_, 5, 0);
    lv_obj_set_style_border_width(settings_crypto_selector_, 1, 0);
    lv_obj_set_flex_grow(settings_crypto_selector_, 1);
    lv_obj_set_scrollbar_mode(settings_crypto_selector_, LV_SCROLLBAR_MODE_AUTO);
    
    // 创建时间框架选择区域
    settings_timeframe_selector_ = lv_obj_create(settings_page_);
    lv_obj_set_size(settings_timeframe_selector_, width_ - 20, 90);
    lv_obj_set_style_pad_all(settings_timeframe_selector_, 5, 0);
    lv_obj_set_style_border_width(settings_timeframe_selector_, 1, 0);
    lv_obj_set_flex_grow(settings_timeframe_selector_, 1);
    lv_obj_set_scrollbar_mode(settings_timeframe_selector_, LV_SCROLLBAR_MODE_AUTO);
    
    // 添加标题
    lv_obj_t* title = lv_label_create(settings_header_);
    lv_label_set_text(title, "Settings");
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 0, 0);
    
    // 添加主题选择标题
    lv_obj_t* theme_title = lv_label_create(settings_theme_selector_);
    lv_label_set_text(theme_title, "Theme Style:");
    lv_obj_align(theme_title, LV_ALIGN_TOP_LEFT, 0, 0);
    
    // 添加虚拟币选择标题
    lv_obj_t* crypto_title = lv_label_create(settings_crypto_selector_);
    lv_label_set_text(crypto_title, "Cryptocurrencies:");
    lv_obj_align(crypto_title, LV_ALIGN_TOP_LEFT, 0, 0);
    
    // 添加时间框架选择标题
    lv_obj_t* timeframe_title = lv_label_create(settings_timeframe_selector_);
    lv_label_set_text(timeframe_title, "Timeframes:");
    lv_obj_align(timeframe_title, LV_ALIGN_TOP_LEFT, 0, 0);
}

void WXT185Display::CreateScreensaverPage() {
    screensaver_page_ = lv_obj_create(page_view_);
    lv_obj_set_size(screensaver_page_, width_, height_);
    lv_obj_set_style_pad_all(screensaver_page_, 0, 0);
    lv_obj_set_style_border_width(screensaver_page_, 0, 0);
    lv_obj_set_style_bg_opa(screensaver_page_, LV_OPA_TRANSP, 0);
    
    // 添加触摸事件处理
    lv_obj_add_event_cb(screensaver_page_, TouchEventHandler, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(screensaver_page_, TouchEventHandler, LV_EVENT_RELEASED, this);
    
    // 创建屏保容器
    screensaver_container_ = lv_obj_create(screensaver_page_);
    lv_obj_set_size(screensaver_container_, width_ - 40, height_ - 40);
    lv_obj_set_style_pad_all(screensaver_container_, 20, 0);
    lv_obj_set_style_border_width(screensaver_container_, 0, 0);
    lv_obj_set_style_radius(screensaver_container_, 15, 0);
    lv_obj_center(screensaver_container_);
    
    // 创建虚拟币名称标签
    screensaver_crypto_name_ = lv_label_create(screensaver_container_);
    lv_label_set_text(screensaver_crypto_name_, "Bitcoin");
    lv_obj_set_style_text_font(screensaver_crypto_name_, &lv_font_montserrat_20, 0);
    lv_obj_align(screensaver_crypto_name_, LV_ALIGN_TOP_MID, 0, 20);
    
    // 创建价格标签
    screensaver_crypto_price_ = lv_label_create(screensaver_container_);
    lv_label_set_text(screensaver_crypto_price_, "$45,000.00");
    lv_obj_set_style_text_font(screensaver_crypto_price_, &lv_font_montserrat_24, 0);
    lv_obj_align(screensaver_crypto_price_, LV_ALIGN_TOP_MID, 0, 60);
    
    // 创建涨跌幅标签
    screensaver_crypto_change_ = lv_label_create(screensaver_container_);
    lv_label_set_text(screensaver_crypto_change_, "+2.50%");
    lv_obj_set_style_text_font(screensaver_crypto_change_, &lv_font_montserrat_16, 0);
    lv_obj_align(screensaver_crypto_change_, LV_ALIGN_TOP_MID, 0, 100);
    
    // 创建时间标签
    screensaver_time_ = lv_label_create(screensaver_container_);
    lv_label_set_text(screensaver_time_, "12:00:00");
    lv_obj_set_style_text_font(screensaver_time_, &lv_font_montserrat_16, 0);
    lv_obj_align(screensaver_time_, LV_ALIGN_BOTTOM_MID, 0, -20);
    
    // 初始隐藏屏保页面
    lv_obj_add_flag(screensaver_page_, LV_OBJ_FLAG_HIDDEN);
}

void WXT185Display::ApplyTheme() {
    DisplayLockGuard lock(this);
    
    // 应用主屏幕背景色
    lv_obj_set_style_bg_color(main_screen_, current_wxt185_theme_.background, 0);
    
    ApplyChatPageTheme();
    ApplyCryptoPageTheme();
    ApplySettingsPageTheme();
    ApplyScreensaverTheme(); // 应用屏保主题
}

void WXT185Display::ApplyChatPageTheme() {
    if (!chat_page_ || !chat_status_bar_ || !chat_content_) return;
    
    // 应用聊天页面主题
    lv_obj_set_style_bg_color(chat_page_, current_wxt185_theme_.background, 0);
    lv_obj_set_style_bg_opa(chat_page_, LV_OPA_COVER, 0);
    
    // 应用状态栏主题
    lv_obj_set_style_bg_color(chat_status_bar_, current_wxt185_theme_.header, 0);
    lv_obj_set_style_text_color(chat_status_bar_, current_wxt185_theme_.text, 0);
    
    // 应用内容区域主题
    lv_obj_set_style_bg_color(chat_content_, current_wxt185_theme_.chat_background, 0);
    lv_obj_set_style_border_color(chat_content_, current_wxt185_theme_.border, 0);
    
    // 应用输入区域主题
    if (chat_input_area_) {
        lv_obj_set_style_bg_color(chat_input_area_, current_wxt185_theme_.selector, 0);
        lv_obj_set_style_border_color(chat_input_area_, current_wxt185_theme_.border, 0);
    }
}

void WXT185Display::ApplyCryptoPageTheme() {
    if (!crypto_page_ || !crypto_header_ || !crypto_chart_ || !crypto_list_) return;
    
    // 应用虚拟币页面主题
    lv_obj_set_style_bg_color(crypto_page_, current_wxt185_theme_.background, 0);
    
    // 应用头部区域主题
    lv_obj_set_style_bg_color(crypto_header_, current_wxt185_theme_.header, 0);
    lv_obj_set_style_text_color(crypto_header_, current_wxt185_theme_.text, 0);
    
    // 应用图表区域主题
    lv_obj_set_style_bg_color(crypto_chart_, current_wxt185_theme_.selector, 0);
    lv_obj_set_style_border_color(crypto_chart_, current_wxt185_theme_.border, 0);
    
    // 应用列表区域主题
    lv_obj_set_style_bg_color(crypto_list_, current_wxt185_theme_.selector, 0);
    lv_obj_set_style_border_color(crypto_list_, current_wxt185_theme_.border, 0);
    
    // 应用时间选择器主题
    if (crypto_time_selector_) {
        lv_obj_set_style_bg_color(crypto_time_selector_, current_wxt185_theme_.selector, 0);
        lv_obj_set_style_border_color(crypto_time_selector_, current_wxt185_theme_.border, 0);
    }
}

void WXT185Display::ApplySettingsPageTheme() {
    if (!settings_page_ || !settings_header_ || !settings_theme_selector_ || 
        !settings_crypto_selector_ || !settings_timeframe_selector_) return;
    
    // 应用设置页面主题
    lv_obj_set_style_bg_color(settings_page_, current_wxt185_theme_.background, 0);
    
    // 应用头部区域主题
    lv_obj_set_style_bg_color(settings_header_, current_wxt185_theme_.header, 0);
    lv_obj_set_style_text_color(settings_header_, current_wxt185_theme_.text, 0);
    
    // 应用主题选择区域主题
    lv_obj_set_style_bg_color(settings_theme_selector_, current_wxt185_theme_.selector, 0);
    lv_obj_set_style_border_color(settings_theme_selector_, current_wxt185_theme_.border, 0);
    
    // 应用虚拟币选择区域主题
    lv_obj_set_style_bg_color(settings_crypto_selector_, current_wxt185_theme_.selector, 0);
    lv_obj_set_style_border_color(settings_crypto_selector_, current_wxt185_theme_.border, 0);
    
    // 应用时间框架选择区域主题
    lv_obj_set_style_bg_color(settings_timeframe_selector_, current_wxt185_theme_.selector, 0);
    lv_obj_set_style_border_color(settings_timeframe_selector_, current_wxt185_theme_.border, 0);
}

void WXT185Display::ApplyScreensaverTheme() {
    if (!screensaver_page_ || !screensaver_container_) return;
    
    // 应用屏保页面主题
    lv_obj_set_style_bg_color(screensaver_page_, current_wxt185_theme_.background, 0);
    
    // 应用屏保容器主题
    lv_obj_set_style_bg_color(screensaver_container_, current_wxt185_theme_.header, 0);
    lv_obj_set_style_bg_opa(screensaver_container_, LV_OPA_90, 0);
    
    // 应用文本颜色
    if (screensaver_crypto_name_) {
        lv_obj_set_style_text_color(screensaver_crypto_name_, current_wxt185_theme_.text, 0);
    }
    
    if (screensaver_crypto_price_) {
        lv_obj_set_style_text_color(screensaver_crypto_price_, current_wxt185_theme_.text, 0);
    }
    
    if (screensaver_crypto_change_) {
        lv_obj_set_style_text_color(screensaver_crypto_change_, current_wxt185_theme_.text, 0);
    }
    
    if (screensaver_time_) {
        lv_obj_set_style_text_color(screensaver_time_, current_wxt185_theme_.text, 0);
    }
}

void WXT185Display::SetEmotion(const char* emotion) {
    DisplayLockGuard lock(this);
    if (emotion_label_ != nullptr) {
        lv_label_set_text(emotion_label_, emotion);
    }
}

void WXT185Display::SetIcon(const char* icon) {
    DisplayLockGuard lock(this);
    if (emotion_label_ != nullptr) {
        lv_label_set_text(emotion_label_, icon);
    }
}

void WXT185Display::SetPreviewImage(const lv_img_dsc_t* img_dsc) {
    // 在这个UI中暂时不实现预览图像功能
    SpiLcdDisplay::SetPreviewImage(img_dsc);
}

void WXT185Display::SetChatMessage(const char* role, const char* content) {
    // 用户活动时更新活动时间
    OnActivity();
    
    DisplayLockGuard lock(this);
    if (chat_content_ == nullptr) return;
    
    // 创建消息气泡
    lv_obj_t* msg_bubble = lv_obj_create(chat_content_);
    lv_obj_set_style_radius(msg_bubble, 8, 0);
    lv_obj_set_scrollbar_mode(msg_bubble, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(msg_bubble, 1, 0);
    lv_obj_set_style_border_color(msg_bubble, current_wxt185_theme_.border, 0);
    lv_obj_set_style_pad_all(msg_bubble, 8, 0);

    // 创建消息文本
    lv_obj_t* msg_text = lv_label_create(msg_bubble);
    lv_label_set_text(msg_text, content);
    
    // 计算文本实际宽度
    lv_coord_t text_width = lv_txt_get_width(content, strlen(content), fonts_.text_font, 0);

    // 计算气泡宽度（为360*360屏幕优化）
    lv_coord_t max_width = width_ * 75 / 100 - 16;  // 屏幕宽度的75%
    lv_coord_t min_width = 20;  
    lv_coord_t bubble_width;
    
    // 确保文本宽度不小于最小宽度
    if (text_width < min_width) {
        text_width = min_width;
    }

    // 如果文本宽度小于最大宽度，使用文本宽度
    if (text_width < max_width) {
        bubble_width = text_width; 
    } else {
        bubble_width = max_width;
    }
    
    // 设置消息文本的宽度
    lv_obj_set_width(msg_text, bubble_width);
    lv_label_set_long_mode(msg_text, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(msg_text, fonts_.text_font, 0);

    // 设置气泡宽度和高度
    lv_obj_set_width(msg_bubble, bubble_width);
    lv_obj_set_height(msg_bubble, LV_SIZE_CONTENT);

    // 根据消息角色设置样式和对齐方式
    if (strcmp(role, "user") == 0) {
        // 用户消息右对齐，绿色背景
        lv_obj_set_style_bg_color(msg_bubble, current_wxt185_theme_.user_bubble, 0);
        lv_obj_set_style_text_color(msg_text, current_wxt185_theme_.text, 0);
        lv_obj_set_user_data(msg_bubble, (void*)"user");
        lv_obj_set_width(msg_bubble, LV_SIZE_CONTENT);
        lv_obj_set_height(msg_bubble, LV_SIZE_CONTENT);
        lv_obj_set_style_flex_grow(msg_bubble, 0, 0);
    } else if (strcmp(role, "assistant") == 0) {
        // 助手消息左对齐，灰色背景
        lv_obj_set_style_bg_color(msg_bubble, current_wxt185_theme_.assistant_bubble, 0);
        lv_obj_set_style_text_color(msg_text, current_wxt185_theme_.text, 0);
        lv_obj_set_user_data(msg_bubble, (void*)"assistant");
        lv_obj_set_width(msg_bubble, LV_SIZE_CONTENT);
        lv_obj_set_height(msg_bubble, LV_SIZE_CONTENT);
        lv_obj_set_style_flex_grow(msg_bubble, 0, 0);
    } else if (strcmp(role, "system") == 0) {
        // 系统消息居中对齐，浅灰色背景
        lv_obj_set_style_bg_color(msg_bubble, current_wxt185_theme_.system_bubble, 0);
        lv_obj_set_style_text_color(msg_text, current_wxt185_theme_.system_text, 0);
        lv_obj_set_user_data(msg_bubble, (void*)"system");
        lv_obj_set_width(msg_bubble, LV_SIZE_CONTENT);
        lv_obj_set_height(msg_bubble, LV_SIZE_CONTENT);
        lv_obj_set_style_flex_grow(msg_bubble, 0, 0);
    }
    
    // 为用户消息创建全宽容器以确保右对齐
    if (strcmp(role, "user") == 0) {
        // 创建全宽容器
        lv_obj_t* container = lv_obj_create(chat_content_);
        lv_obj_set_width(container, width_ - 20);
        lv_obj_set_height(container, LV_SIZE_CONTENT);
        
        // 使容器透明且无边框
        lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(container, 0, 0);
        lv_obj_set_style_pad_all(container, 0, 0);
        
        // 将消息气泡移入此容器
        lv_obj_set_parent(msg_bubble, container);
        
        // 右对齐气泡在容器中
        lv_obj_align(msg_bubble, LV_ALIGN_RIGHT_MID, -10, 0);
        
        // 自动滚动到此容器
        lv_obj_scroll_to_view_recursive(container, LV_ANIM_ON);
    } else if (strcmp(role, "system") == 0) {
        // 为系统消息创建全宽容器以确保居中对齐
        lv_obj_t* container = lv_obj_create(chat_content_);
        lv_obj_set_width(container, width_ - 20);
        lv_obj_set_height(container, LV_SIZE_CONTENT);
        
        // 使容器透明且无边框
        lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(container, 0, 0);
        lv_obj_set_style_pad_all(container, 0, 0);
        
        // 将消息气泡移入此容器
        lv_obj_set_parent(msg_bubble, container);
        
        // 将气泡居中对齐在容器中
        lv_obj_align(msg_bubble, LV_ALIGN_CENTER, 0, 0);
        
        // 自动滚动底部
        lv_obj_scroll_to_view_recursive(container, LV_ANIM_ON);
    } else {
        // 助手消息左对齐
        lv_obj_align(msg_bubble, LV_ALIGN_LEFT_MID, 0, 0);
        // 自动滚动到消息气泡
        lv_obj_scroll_to_view_recursive(msg_bubble, LV_ANIM_ON);
    }
    
    // 存储对最新消息标签的引用
    chat_message_label_ = msg_text;
}

void WXT185Display::SetTheme(const std::string& theme_name) {
    DisplayLockGuard lock(this);
    
    // 先调用父类方法设置基础主题
    SpiLcdDisplay::SetTheme(theme_name);
    
    // 然后处理自定义主题
    if (theme_name == "dark" || theme_name == "DARK") {
        current_theme_style_ = ThemeStyle::DARK;
        current_wxt185_theme_ = DARK_THEME_WXT185;
    } else if (theme_name == "light" || theme_name == "LIGHT") {
        current_theme_style_ = ThemeStyle::LIGHT;
        current_wxt185_theme_ = LIGHT_THEME_WXT185;
    } else if (theme_name == "metal") {
        current_theme_style_ = ThemeStyle::METAL;
        current_wxt185_theme_ = METAL_THEME_WXT185;
    } else if (theme_name == "technology") {
        current_theme_style_ = ThemeStyle::TECHNOLOGY;
        current_wxt185_theme_ = TECHNOLOGY_THEME_WXT185;
    } else if (theme_name == "cosmic") {
        current_theme_style_ = ThemeStyle::COSMIC;
        current_wxt185_theme_ = COSMIC_THEME_WXT185;
    }
    
    ApplyTheme();
}

void WXT185Display::AddCryptocurrency(const CryptocurrencyData& crypto) {
    crypto_data_.push_back(crypto);
}

void WXT185Display::RemoveCryptocurrency(const std::string& symbol) {
    crypto_data_.erase(
        std::remove_if(crypto_data_.begin(), crypto_data_.end(),
                      [&symbol](const CryptocurrencyData& item) {
                          return item.symbol == symbol;
                      }),
        crypto_data_.end());
}

void WXT185Display::UpdateCryptocurrencyPrice(const std::string& symbol, float price, float change) {
    for (auto& crypto : crypto_data_) {
        if (crypto.symbol == symbol) {
            crypto.price = price;
            crypto.change_24h = change;
            break;
        }
    }
    
    UpdateCryptoData();
}

void WXT185Display::UpdateCryptoData() {
    DisplayLockGuard lock(this);
    if (crypto_list_ == nullptr) return;
    
    // 清除现有列表项
    lv_obj_clean(crypto_list_);
    
    // 添加虚拟币列表项
    for (const auto& crypto : crypto_data_) {
        lv_obj_t* item = lv_obj_create(crypto_list_);
        lv_obj_set_size(item, width_ - 40, 35);
        lv_obj_set_style_pad_all(item, 5, 0);
        lv_obj_set_style_radius(item, 5, 0);
        lv_obj_set_style_bg_color(item, current_wxt185_theme_.background, 0);
        lv_obj_set_style_border_width(item, 1, 0);
        lv_obj_set_style_border_color(item, current_wxt185_theme_.border, 0);
        
        // 名称
        lv_obj_t* symbol_label = lv_label_create(item);
        lv_label_set_text(symbol_label, crypto.symbol.c_str());
        lv_obj_align(symbol_label, LV_ALIGN_LEFT_MID, 0, 0);
        lv_obj_set_style_text_color(symbol_label, current_wxt185_theme_.text, 0);
        
        // 价格
        lv_obj_t* price_label = lv_label_create(item);
        char price_text[32];
        snprintf(price_text, sizeof(price_text), "$%.2f", crypto.price);
        lv_label_set_text(price_label, price_text);
        lv_obj_align(price_label, LV_ALIGN_RIGHT_MID, -50, 0);
        lv_obj_set_style_text_color(price_label, current_wxt185_theme_.text, 0);
        
        // 变化率
        lv_obj_t* change_label = lv_label_create(item);
        char change_text[32];
        snprintf(change_text, sizeof(change_text), "%.2f%%", crypto.change_24h);
        lv_label_set_text(change_label, change_text);
        lv_obj_align(change_label, LV_ALIGN_RIGHT_MID, 0, 0);
        lv_obj_set_style_text_color(change_label, current_wxt185_theme_.text, 0);
        
        if (crypto.change_24h >= 0) {
            lv_obj_set_style_text_color(change_label, lv_color_hex(0x00FF00), 0);
        } else {
            lv_obj_set_style_text_color(change_label, lv_color_hex(0xFF0000), 0);
        }
    }
}

void WXT185Display::DrawKLineChart() {
    DisplayLockGuard lock(this);
    if (crypto_chart_ == nullptr) return;
    
    // 清除现有图表
    lv_obj_clean(crypto_chart_);
    
    // 绘制K线图占位符
    lv_obj_t* chart_title = lv_label_create(crypto_chart_);
    lv_label_set_text(chart_title, "K-Line Chart (Placeholder)");
    lv_obj_set_style_text_color(chart_title, current_wxt185_theme_.text, 0);
    lv_obj_center(chart_title);
}

void WXT185Display::TouchEventHandler(lv_event_t* e) {
    WXT185Display* self = static_cast<WXT185Display*>(lv_event_get_user_data(e));
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* target = lv_event_get_target(e);
    
    // 触摸事件视为用户活动
    self->OnActivity();
    
    if (code == LV_EVENT_PRESSED) {
        // 记录触摸开始点
        lv_indev_t* indev = lv_indev_get_act();
        if (indev) {
            lv_point_t point;
            lv_indev_get_point(indev, &point);
            self->HandleTouchStart(point);
        }
    } else if (code == LV_EVENT_RELEASED) {
        // 处理触摸释放
        lv_indev_t* indev = lv_indev_get_act();
        if (indev) {
            lv_point_t point;
            lv_indev_get_point(indev, &point);
            self->HandleTouchEnd(point);
        }
    }
}

void WXT185Display::OnWakeWordDetected() {
    // 检测到唤醒词时退出屏保
    OnActivity();
}

void WXT185Display::HandleTouchStart(lv_point_t point) {
    touch_start_point_ = point;
    is_touching_ = true;
}

void WXT185Display::HandleTouchEnd(lv_point_t point) {
    if (!is_touching_) return;
    
    is_touching_ = false;
    
    // 计算滑动距离
    int16_t diff_x = point.x - touch_start_point_.x;
    int16_t diff_y = point.y - touch_start_point_.y;
    
    // 判断是否为水平滑动且距离足够
    if (abs(diff_x) > abs(diff_y) && abs(diff_x) > 30) {
        if (diff_x > 0) {
            // 向右滑动，切换到上一个页面
            if (current_page_index_ > 0) {
                SwitchToPage(current_page_index_ - 1);
            }
        } else {
            // 向左滑动，切换到下一个页面
            if (current_page_index_ < 2) {
                SwitchToPage(current_page_index_ + 1);
            }
        }
    }
}

void WXT185Display::SwitchToPage(int page_index) {
    DisplayLockGuard lock(this);
    if (page_view_ == nullptr || page_index < 0 || page_index > 2) return;
    
    current_page_index_ = page_index;
    
    // 滚动到指定页面
    lv_obj_t* target_page = nullptr;
    switch (page_index) {
        case 0:
            target_page = chat_page_;
            break;
        case 1:
            target_page = crypto_page_;
            break;
        case 2:
            target_page = settings_page_;
            break;
        default:
            return;
    }
    
    if (target_page) {
        lv_obj_scroll_to_view_recursive(target_page, LV_ANIM_ON);
    }
}

void WXT185Display::PageEventHandler(lv_event_t* e) {
    // 页面事件处理
}

void WXT185Display::CryptoSelectorEventHandler(lv_event_t* e) {
    // 虚拟币选择事件处理
}

void WXT185Display::ThemeSelectorEventHandler(lv_event_t* e) {
    // 主题选择事件处理
}

void WXT185Display::TimeframeSelectorEventHandler(lv_event_t* e) {
    // 时间框架选择事件处理
}

void WXT185Display::ScreensaverTimerCallback(void* arg) {
    WXT185Display* self = static_cast<WXT185Display*>(arg);
    
    // 检查是否超时
    int64_t current_time = esp_timer_get_time() / 1000; // 转换为毫秒
    if (current_time - self->last_activity_time_ >= SCREENSAVER_TIMEOUT_MS) {
        // 进入屏保模式
        self->EnterScreensaver();
    }
}

void WXT185Display::StartScreensaverTimer() {
    if (screensaver_timer_) {
        esp_timer_stop(screensaver_timer_);
        esp_timer_start_periodic(screensaver_timer_, 1000000); // 每秒检查一次
    }
}

void WXT185Display::StopScreensaverTimer() {
    if (screensaver_timer_) {
        esp_timer_stop(screensaver_timer_);
    }
}

void WXT185Display::EnterScreensaver() {
    DisplayLockGuard lock(this);
    
    if (screensaver_active_ || !screensaver_page_) return;
    
    screensaver_active_ = true;
    
    // 隐藏当前页面
    lv_obj_add_flag(chat_page_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(crypto_page_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(settings_page_, LV_OBJ_FLAG_HIDDEN);
    
    // 显示屏保页面
    lv_obj_clear_flag(screensaver_page_, LV_OBJ_FLAG_HIDDEN);
    
    // 更新屏保内容
    UpdateScreensaverContent();
    
    ESP_LOGI(TAG, "Entered screensaver mode");
}

void WXT185Display::ExitScreensaver() {
    DisplayLockGuard lock(this);
    
    if (!screensaver_active_ || !screensaver_page_) return;
    
    screensaver_active_ = false;
    
    // 隐藏屏保页面
    lv_obj_add_flag(screensaver_page_, LV_OBJ_FLAG_HIDDEN);
    
    // 显示当前页面
    switch (current_page_index_) {
        case 0:
            lv_obj_clear_flag(chat_page_, LV_OBJ_FLAG_HIDDEN);
            break;
        case 1:
            lv_obj_clear_flag(crypto_page_, LV_OBJ_FLAG_HIDDEN);
            break;
        case 2:
            lv_obj_clear_flag(settings_page_, LV_OBJ_FLAG_HIDDEN);
            break;
    }
    
    ESP_LOGI(TAG, "Exited screensaver mode");
}

void WXT185Display::UpdateScreensaverContent() {
    DisplayLockGuard lock(this);
    
    if (!screensaver_active_ || crypto_data_.empty()) return;
    
    // 使用第一个虚拟币数据作为屏保显示内容
    const CryptocurrencyData& crypto = crypto_data_[0];
    
    // 更新虚拟币名称
    if (screensaver_crypto_name_) {
        lv_label_set_text(screensaver_crypto_name_, crypto.name.c_str());
    }
    
    // 更新价格
    if (screensaver_crypto_price_) {
        char price_text[32];
        snprintf(price_text, sizeof(price_text), "$%.2f", crypto.price);
        lv_label_set_text(screensaver_crypto_price_, price_text);
    }
    
    // 更新涨跌幅
    if (screensaver_crypto_change_) {
        char change_text[32];
        snprintf(change_text, sizeof(change_text), "%.2f%%", crypto.change_24h);
        lv_label_set_text(screensaver_crypto_change_, change_text);
        
        // 根据涨跌设置颜色
        if (crypto.change_24h >= 0) {
            lv_obj_set_style_text_color(screensaver_crypto_change_, lv_color_hex(0x00FF00), 0); // 绿色
        } else {
            lv_obj_set_style_text_color(screensaver_crypto_change_, lv_color_hex(0xFF0000), 0); // 红色
        }
    }
    
    // 更新时间
    if (screensaver_time_) {
        time_t now;
        time(&now);
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);
        
        char time_str[32];
        strftime(time_str, sizeof(time_str), "%H:%M:%S", &timeinfo);
        lv_label_set_text(screensaver_time_, time_str);
    }
}

void WXT185Display::OnActivity() {
    // 更新最后活动时间
    last_activity_time_ = esp_timer_get_time() / 1000; // 转换为毫秒
    
    // 如果当前处于屏保状态，则退出屏保
    if (screensaver_active_) {
        ExitScreensaver();
    }
}

void WXT185Display::OnConversationStart() {
    // 对话开始时视为用户活动
    OnActivity();
}

void WXT185Display::OnConversationEnd() {
    // 对话结束时更新活动时间，10秒后可能进入屏保
    last_activity_time_ = esp_timer_get_time() / 1000; // 转换为毫秒
}

void WXT185Display::OnIdle() {
    // 空闲状态时更新活动时间，10秒后可能进入屏保
    last_activity_time_ = esp_timer_get_time() / 1000; // 转换为毫秒
}

void WXT185Display::OnDeviceStateChanged(int previous_state, int current_state) {
    // 根据设备状态变化控制屏保
    switch (current_state) {
        case 3: // kDeviceStateIdle
            // 设备进入空闲状态，设置屏保计时器
            OnIdle();
            break;
            
        case 6: // kDeviceStateListening
        case 7: // kDeviceStateSpeaking
            // 设备开始对话，视为用户活动
            OnConversationStart();
            break;
            
        default:
            // 其他状态变化也视为用户活动
            OnActivity();
            break;
    }
}
