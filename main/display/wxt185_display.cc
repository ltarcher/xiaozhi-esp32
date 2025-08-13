#include "wxt185_display.h"
#include "lvgl.h"
#include <esp_log.h>
#include <algorithm>
#include <ctime>
#include "assets/lang_config.h"
#include "device_state.h"
#include <font_awesome_symbols.h>
#include <esp_lvgl_port.h>
#include "settings.h"
#include "application.h"

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
#define LIGHT_OUTER_RING_COLOR       lv_color_hex(0xDDDDDD)  // 外圆环颜色
#define LIGHT_INNER_RING_COLOR       lv_color_hex(0xEEEEEE)  // 内圆环颜色
#define LIGHT_SCREENSAVER_SWITCH_COLOR lv_color_hex(0x1A6C37) // 屏保开关颜色

// 虚拟币界面颜色定义 - 基于LIGHT主题
#define LIGHT_CRYPTO_BACKGROUND_COLOR      LIGHT_BACKGROUND_COLOR      // 背景色
#define LIGHT_CRYPTO_TEXT_COLOR            LIGHT_TEXT_COLOR            // 主文本色
#define LIGHT_CRYPTO_SUB_TEXT_COLOR        LIGHT_SYSTEM_TEXT_COLOR     // 次要文本色
#define LIGHT_CRYPTO_UP_COLOR              LIGHT_TEXT_COLOR            // 上涨颜色
#define LIGHT_CRYPTO_DOWN_COLOR            LIGHT_LOW_BATTERY_COLOR     // 下跌颜色
#define LIGHT_CRYPTO_BORDER_COLOR          LIGHT_BORDER_COLOR          // 边框颜色
#define LIGHT_CRYPTO_PROGRESS_BG_COLOR     LIGHT_CHAT_BACKGROUND_COLOR // 进度环背景色

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
#define DARK_OUTER_RING_COLOR        lv_color_hex(0x252525)  // 外圆环颜色
#define DARK_INNER_RING_COLOR        lv_color_hex(0x1E1E1E)  // 内圆环颜色
#define DARK_SCREENSAVER_SWITCH_COLOR lv_color_hex(0x1A6C37)  // 屏保开关颜色

// 虚拟币界面颜色定义 - 基于DARK主题
#define DARK_CRYPTO_BACKGROUND_COLOR      DARK_BACKGROUND_COLOR      // 背景色
#define DARK_CRYPTO_TEXT_COLOR            DARK_TEXT_COLOR            // 主文本色
#define DARK_CRYPTO_SUB_TEXT_COLOR        DARK_SYSTEM_TEXT_COLOR     // 次要文本色
#define DARK_CRYPTO_UP_COLOR              DARK_TEXT_COLOR            // 上涨颜色
#define DARK_CRYPTO_DOWN_COLOR            DARK_LOW_BATTERY_COLOR     // 下跌颜色(红色)
#define DARK_CRYPTO_BORDER_COLOR          DARK_BORDER_COLOR          // 边框颜色
#define DARK_CRYPTO_PROGRESS_BG_COLOR     DARK_CHAT_BACKGROUND_COLOR // 进度环背景色

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
#define METAL_OUTER_RING_COLOR       lv_color_hex(0xB0B0B0)   // 外圆环颜色
#define METAL_INNER_RING_COLOR       lv_color_hex(0xC0C0C0)   // 内圆环颜色
#define METAL_SCREENSAVER_SWITCH_COLOR lv_color_hex(0x1A6C37)  // 屏保开关颜色

// 虚拟币界面颜色定义 - 基于METAL主题
#define METAL_CRYPTO_BACKGROUND_COLOR      METAL_BACKGROUND_COLOR      // 背景色
#define METAL_CRYPTO_TEXT_COLOR            METAL_TEXT_COLOR            // 主文本色
#define METAL_CRYPTO_SUB_TEXT_COLOR        METAL_SYSTEM_TEXT_COLOR     // 次要文本色
#define METAL_CRYPTO_UP_COLOR              METAL_TEXT_COLOR            // 上涨颜色
#define METAL_CRYPTO_DOWN_COLOR            METAL_LOW_BATTERY_COLOR     // 下跌颜色(红色)
#define METAL_CRYPTO_BORDER_COLOR          METAL_BORDER_COLOR          // 边框颜色
#define METAL_CRYPTO_PROGRESS_BG_COLOR     METAL_CHAT_BACKGROUND_COLOR // 进度环背景色

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
#define TECHNOLOGY_OUTER_RING_COLOR  lv_color_hex(0x151535)   // 外圆环颜色
#define TECHNOLOGY_INNER_RING_COLOR  lv_color_hex(0x0A0A20)   // 内圆环颜色
#define TECHNOLOGY_SCREENSAVER_SWITCH_COLOR lv_color_hex(0x006400) // 屏保开关颜色

// 虚拟币界面颜色定义 - 基于TECHNOLOGY主题
#define TECHNOLOGY_CRYPTO_BACKGROUND_COLOR      TECHNOLOGY_BACKGROUND_COLOR      // 背景色
#define TECHNOLOGY_CRYPTO_TEXT_COLOR            TECHNOLOGY_TEXT_COLOR            // 主文本色
#define TECHNOLOGY_CRYPTO_SUB_TEXT_COLOR        TECHNOLOGY_SYSTEM_TEXT_COLOR     // 次要文本色
#define TECHNOLOGY_CRYPTO_UP_COLOR              TECHNOLOGY_TEXT_COLOR            // 上涨颜色(青色)
#define TECHNOLOGY_CRYPTO_DOWN_COLOR            TECHNOLOGY_LOW_BATTERY_COLOR     // 下跌颜色(橙红色)
#define TECHNOLOGY_CRYPTO_BORDER_COLOR          TECHNOLOGY_BORDER_COLOR          // 边框颜色
#define TECHNOLOGY_CRYPTO_PROGRESS_BG_COLOR     TECHNOLOGY_CHAT_BACKGROUND_COLOR // 进度环背景色

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
#define COSMIC_OUTER_RING_COLOR      lv_color_hex(0x150015)   // 外圆环颜色
#define COSMIC_INNER_RING_COLOR      lv_color_hex(0x100010)   // 内圆环颜色
#define COSMIC_SCREENSAVER_SWITCH_COLOR lv_color_hex(0x4B0082) // 屏保开关颜色

// 虚拟币界面颜色定义 - 基于COSMIC主题
#define COSMIC_CRYPTO_BACKGROUND_COLOR      COSMIC_BACKGROUND_COLOR      // 背景色
#define COSMIC_CRYPTO_TEXT_COLOR            COSMIC_TEXT_COLOR            // 主文本色
#define COSMIC_CRYPTO_SUB_TEXT_COLOR        COSMIC_SYSTEM_TEXT_COLOR     // 次要文本色
#define COSMIC_CRYPTO_UP_COLOR              COSMIC_TEXT_COLOR            // 上涨颜色(黄色)
#define COSMIC_CRYPTO_DOWN_COLOR            COSMIC_LOW_BATTERY_COLOR     // 下跌颜色(红色)
#define COSMIC_CRYPTO_BORDER_COLOR          COSMIC_BORDER_COLOR          // 边框颜色
#define COSMIC_CRYPTO_PROGRESS_BG_COLOR     COSMIC_CHAT_BACKGROUND_COLOR // 进度环背景色

// 主题定义
const WXT185ThemeColors LIGHT_THEME_WXT185 = {
    .background = LIGHT_BACKGROUND_COLOR,
    .text = LIGHT_TEXT_COLOR,
    .outer_ring_background = LIGHT_OUTER_RING_COLOR,
    .inner_ring_background = LIGHT_INNER_RING_COLOR,
    .chat_background = LIGHT_CHAT_BACKGROUND_COLOR,
    .user_bubble = LIGHT_USER_BUBBLE_COLOR,
    .assistant_bubble = LIGHT_ASSISTANT_BUBBLE_COLOR,
    .system_bubble = LIGHT_SYSTEM_BUBBLE_COLOR,
    .system_text = LIGHT_SYSTEM_TEXT_COLOR,
    .border = LIGHT_BORDER_COLOR,
    .low_battery = LIGHT_LOW_BATTERY_COLOR,
    .header = LIGHT_HEADER_COLOR,
    .selector = LIGHT_SELECTOR_COLOR,
    .crypto_background = LIGHT_CRYPTO_BACKGROUND_COLOR,
    .crypto_text = LIGHT_CRYPTO_TEXT_COLOR,
    .crypto_sub_text = LIGHT_CRYPTO_SUB_TEXT_COLOR,
    .crypto_up_color = LIGHT_CRYPTO_UP_COLOR,
    .crypto_down_color = LIGHT_CRYPTO_DOWN_COLOR,
    .crypto_border_color = LIGHT_CRYPTO_BORDER_COLOR,
    .crypto_progress_bg_color = LIGHT_CRYPTO_PROGRESS_BG_COLOR,
    .settings_screensaver_switch = LIGHT_SCREENSAVER_SWITCH_COLOR
};

const WXT185ThemeColors DARK_THEME_WXT185 = {
    .background = DARK_BACKGROUND_COLOR,
    .text = DARK_TEXT_COLOR,
    .outer_ring_background = DARK_OUTER_RING_COLOR,
    .inner_ring_background = DARK_INNER_RING_COLOR,
    .chat_background = DARK_CHAT_BACKGROUND_COLOR,
    .user_bubble = DARK_USER_BUBBLE_COLOR,
    .assistant_bubble = DARK_ASSISTANT_BUBBLE_COLOR,
    .system_bubble = DARK_SYSTEM_BUBBLE_COLOR,
    .system_text = DARK_SYSTEM_TEXT_COLOR,
    .border = DARK_BORDER_COLOR,
    .low_battery = DARK_LOW_BATTERY_COLOR,
    .header = DARK_HEADER_COLOR,
    .selector = DARK_SELECTOR_COLOR,
    .crypto_background = DARK_CRYPTO_BACKGROUND_COLOR,
    .crypto_text = DARK_CRYPTO_TEXT_COLOR,
    .crypto_sub_text = DARK_CRYPTO_SUB_TEXT_COLOR,
    .crypto_up_color = DARK_CRYPTO_UP_COLOR,
    .crypto_down_color = DARK_CRYPTO_DOWN_COLOR,
    .crypto_border_color = DARK_CRYPTO_BORDER_COLOR,
    .crypto_progress_bg_color = DARK_CRYPTO_PROGRESS_BG_COLOR,
    .settings_screensaver_switch = DARK_SCREENSAVER_SWITCH_COLOR
};

const WXT185ThemeColors METAL_THEME_WXT185 = {
    .background = METAL_BACKGROUND_COLOR,
    .text = METAL_TEXT_COLOR,
    .outer_ring_background = METAL_OUTER_RING_COLOR,
    .inner_ring_background = METAL_INNER_RING_COLOR,
    .chat_background = METAL_CHAT_BACKGROUND_COLOR,
    .user_bubble = METAL_USER_BUBBLE_COLOR,
    .assistant_bubble = METAL_ASSISTANT_BUBBLE_COLOR,
    .system_bubble = METAL_SYSTEM_BUBBLE_COLOR,
    .system_text = METAL_SYSTEM_TEXT_COLOR,
    .border = METAL_BORDER_COLOR,
    .low_battery = METAL_LOW_BATTERY_COLOR,
    .header = METAL_HEADER_COLOR,
    .selector = METAL_SELECTOR_COLOR,
    .crypto_background = METAL_CRYPTO_BACKGROUND_COLOR,
    .crypto_text = METAL_CRYPTO_TEXT_COLOR,
    .crypto_sub_text = METAL_CRYPTO_SUB_TEXT_COLOR,
    .crypto_up_color = METAL_CRYPTO_UP_COLOR,
    .crypto_down_color = METAL_CRYPTO_DOWN_COLOR,
    .crypto_border_color = METAL_CRYPTO_BORDER_COLOR,
    .crypto_progress_bg_color = METAL_CRYPTO_PROGRESS_BG_COLOR,
    .settings_screensaver_switch = METAL_SCREENSAVER_SWITCH_COLOR
};

const WXT185ThemeColors TECHNOLOGY_THEME_WXT185 = {
    .background = TECHNOLOGY_BACKGROUND_COLOR,
    .text = TECHNOLOGY_TEXT_COLOR,
    .outer_ring_background = TECHNOLOGY_OUTER_RING_COLOR,
    .inner_ring_background = TECHNOLOGY_INNER_RING_COLOR,
    .chat_background = TECHNOLOGY_CHAT_BACKGROUND_COLOR,
    .user_bubble = TECHNOLOGY_USER_BUBBLE_COLOR,
    .assistant_bubble = TECHNOLOGY_ASSISTANT_BUBBLE_COLOR,
    .system_bubble = TECHNOLOGY_SYSTEM_BUBBLE_COLOR,
    .system_text = TECHNOLOGY_SYSTEM_TEXT_COLOR,
    .border = TECHNOLOGY_BORDER_COLOR,
    .low_battery = TECHNOLOGY_LOW_BATTERY_COLOR,
    .header = TECHNOLOGY_HEADER_COLOR,
    .selector = TECHNOLOGY_SELECTOR_COLOR,
    .crypto_background = TECHNOLOGY_CRYPTO_BACKGROUND_COLOR,
    .crypto_text = TECHNOLOGY_CRYPTO_TEXT_COLOR,
    .crypto_sub_text = TECHNOLOGY_CRYPTO_SUB_TEXT_COLOR,
    .crypto_up_color = TECHNOLOGY_CRYPTO_UP_COLOR,
    .crypto_down_color = TECHNOLOGY_CRYPTO_DOWN_COLOR,
    .crypto_border_color = TECHNOLOGY_CRYPTO_BORDER_COLOR,
    .crypto_progress_bg_color = TECHNOLOGY_CRYPTO_PROGRESS_BG_COLOR,
    .settings_screensaver_switch = TECHNOLOGY_SCREENSAVER_SWITCH_COLOR
};

const WXT185ThemeColors COSMIC_THEME_WXT185 = {
    .background = COSMIC_BACKGROUND_COLOR,
    .text = COSMIC_TEXT_COLOR,
    .outer_ring_background = COSMIC_OUTER_RING_COLOR,
    .inner_ring_background = COSMIC_INNER_RING_COLOR,
    .chat_background = COSMIC_CHAT_BACKGROUND_COLOR,
    .user_bubble = COSMIC_USER_BUBBLE_COLOR,
    .assistant_bubble = COSMIC_ASSISTANT_BUBBLE_COLOR,
    .system_bubble = COSMIC_SYSTEM_BUBBLE_COLOR,
    .system_text = COSMIC_SYSTEM_TEXT_COLOR,
    .border = COSMIC_BORDER_COLOR,
    .low_battery = COSMIC_LOW_BATTERY_COLOR,
    .header = COSMIC_HEADER_COLOR,
    .selector = COSMIC_SELECTOR_COLOR,
    .crypto_background = COSMIC_CRYPTO_BACKGROUND_COLOR,
    .crypto_text = COSMIC_CRYPTO_TEXT_COLOR,
    .crypto_sub_text = COSMIC_CRYPTO_SUB_TEXT_COLOR,
    .crypto_up_color = COSMIC_CRYPTO_UP_COLOR,
    .crypto_down_color = COSMIC_CRYPTO_DOWN_COLOR,
    .crypto_border_color = COSMIC_CRYPTO_BORDER_COLOR,
    .crypto_progress_bg_color = COSMIC_CRYPTO_PROGRESS_BG_COLOR,
    .settings_screensaver_switch = COSMIC_SCREENSAVER_SWITCH_COLOR    
};

// 定义字体(需要idf.py menuconfig启用lvgl字体配置项)
LV_FONT_DECLARE(lv_font_montserrat_16)
LV_FONT_DECLARE(lv_font_montserrat_24)
LV_FONT_DECLARE(lv_font_montserrat_32)
LV_FONT_DECLARE(lv_font_montserrat_48)

//
LV_FONT_DECLARE(font_awesome_30_4);

// 主题样式字符串
static const char* ThemeString[] = {
    "Light",
    "Dark",
    "Metal",
    "Technology",
    "Cosmic"
};

static uint32_t ThemeCount = sizeof(ThemeString) / sizeof(ThemeString[0]);
#define MAX_THEME_NAME_LENGTH 16

// 设置选项
static int selected_theme = 0;             // 当前选择的主题
static int default_crypto = 0;             // 默认虚拟币
static int kline_frequency = 0;            // K线频率 (0=1分钟, 1=5分钟, 2=15分钟, 3=1小时, 4=4小时, 5=1天, 6=1周, 7=1月, 8=3个月)
static bool screensaver_enabled = true;    // 屏保开关

// 设置主题回调
static void theme_roller_event_handler(lv_event_t * e) {
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        selected_theme = lv_roller_get_selected(obj);
    }
}

// 默认虚拟币选择事件处理函数
static void default_crypto_roller_event_handler(lv_event_t * e) {
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        default_crypto = lv_roller_get_selected(obj);
    }
}

// K线频率选择事件处理函数
static void kline_frequency_roller_event_handler(lv_event_t * e) {
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        kline_frequency = lv_roller_get_selected(obj);
    }
}

// 屏保开关事件处理函数
static void screensaver_switch_event_handler(lv_event_t * e) {
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
    WXT185Display* display = static_cast<WXT185Display*>(lv_event_get_user_data(e));
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        screensaver_enabled = lv_obj_has_state(obj, LV_STATE_CHECKED);
        
        if (screensaver_enabled) {
            // 启用屏保功能
            if (!display->screensaver_timer_) {
                esp_timer_create_args_t timer_args = {
                    .callback = WXT185Display::ScreensaverTimerCallback,
                    .arg = display,
                    .dispatch_method = ESP_TIMER_TASK,
                    .name = "screensaver_timer",
                    .skip_unhandled_events = false,
                };
                esp_timer_create(&timer_args, &display->screensaver_timer_);
            }
            // 启动定时器
            display->StartScreensaverTimer();
        } else {
            // 禁用屏保功能
            if (display->screensaver_timer_) {
                display->StopScreensaverTimer();
            }
            
            // 如果当前正在屏保状态，则退出屏保
            if (display->screensaver_active_) {
                display->ExitScreensaver();
            }
        }
    }
}

void WXT185Display::SaveSettings() {
    // 获取当前设置值
    int theme_index = lv_roller_get_selected(settings_theme_roller_);
    int crypto_index = lv_roller_get_selected(settings_default_crypto_roller_);
    int kline_index = lv_roller_get_selected(settings_kline_time_roller_);
    bool screensaver_state = lv_obj_has_state(settings_screensaver_switch_, LV_STATE_CHECKED);
    
    // 保存设置到NVS
    Settings settings("display", true);
    settings.SetString("theme", ThemeString[theme_index]);
    settings.SetInt("default_crypto", crypto_index);
    settings.SetInt("kline_frequency", kline_index);
    settings.SetInt("screensaver_enabled", screensaver_state ? 1 : 0);
    
    ESP_LOGI(TAG, "Settings saved - Theme: %d, Crypto: %d, KLine: %d, Screensaver: %d", 
             theme_index, crypto_index, kline_index, screensaver_state);
    
    // 显示保存成功的提示信息
    lv_obj_t* save_label = lv_label_create(settings_page_);
    lv_label_set_text(save_label, "Settings saved!");
    lv_obj_set_style_text_color(save_label, lv_color_hex(0x00FF00), 0);
    lv_obj_align(save_label, LV_ALIGN_BOTTOM_MID, 0, -20);
    
    // 3秒后自动删除提示
    struct _timer_data {
        lv_obj_t* label;
    };
    
    _timer_data* data = new _timer_data{save_label};
    
    lv_timer_t* timer = lv_timer_create([](lv_timer_t* timer) {
        _timer_data* data = static_cast<_timer_data*>(lv_timer_get_user_data(timer));
        lv_obj_del(data->label);
        delete data;
        lv_timer_del(timer);
    }, 3000, data);
    
    lv_timer_set_user_data(timer, data);
    
    // 保存当前选择到全局变量，以便在下次启动时使用
    extern int selected_theme;
    extern int default_crypto;
    extern int kline_frequency;
    extern bool screensaver_enabled;
    
    selected_theme = theme_index;
    default_crypto = crypto_index;
    kline_frequency = kline_index;
    screensaver_enabled = screensaver_state;
}

// 保存按钮事件处理函数
static void settings_save_button_event_handler(lv_event_t * e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        WXT185Display* display = static_cast<WXT185Display*>(lv_event_get_user_data(e));
        display->SaveSettings();
    }
}

WXT185Display::WXT185Display(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                           int width, int height, int offset_x, int offset_y,
                           bool mirror_x, bool mirror_y, bool swap_xy,
                           DisplayFonts fonts)
    : LcdDisplay(panel_io, panel, fonts, width, height) {
    ESP_LOGI(TAG, "Initializing WXT185 Display");

    // Load theme from settings
    Settings settings("display", false);
    std::string theme_name = settings.GetString("theme", "light");
    ESP_LOGI(TAG, "Theme: %s", theme_name.c_str());
    if (theme_name == "dark" || theme_name == "DARK") {
        current_wxt185_theme_ = DARK_THEME_WXT185;
    } else if (theme_name == "light" || theme_name == "LIGHT") {
        current_wxt185_theme_ = LIGHT_THEME_WXT185;
    } else if (theme_name == "metal") {
        current_wxt185_theme_ = METAL_THEME_WXT185;
    } else if (theme_name == "technology") {
        current_wxt185_theme_ = TECHNOLOGY_THEME_WXT185;
    } else if (theme_name == "cosmic") {
        current_wxt185_theme_ = COSMIC_THEME_WXT185;
    } else {
        // 默认light
        current_wxt185_theme_ = LIGHT_THEME_WXT185;
    }
    
    // 从设置中加载配置值
    selected_theme = settings.GetInt("theme", 0);             // 默认主题索引
    default_crypto = settings.GetInt("default_crypto", 0);    // 默认虚拟币索引
    kline_frequency = settings.GetInt("kline_frequency", 3);  // 默认K线频率 (3=1小时)
    screensaver_enabled = settings.GetInt("screensaver_enabled", 1) == 1; // 默认启用屏保

    // draw white
    std::vector<uint16_t> buffer(width_, 0xFFFF);
    for (int y = 0; y < height_; y++) {
        esp_lcd_panel_draw_bitmap(panel_, 0, y, width_, y + 1, buffer.data());
    }

    // Set the display to on
    ESP_LOGI(TAG, "Turning display on");
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_, true));

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();

    ESP_LOGI(TAG, "Initialize LVGL port");
    lvgl_port_cfg_t port_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    port_cfg.task_priority = 1;
    port_cfg.timer_period_ms = 50;
    lvgl_port_init(&port_cfg);

    ESP_LOGI(TAG, "Adding LCD display");
    const lvgl_port_display_cfg_t display_cfg = {
        .io_handle = panel_io_,
        .panel_handle = panel_,
        .control_handle = nullptr,
        .buffer_size = static_cast<uint32_t>(width_ * 20),
        .double_buffer = false,
        .trans_size = 0,
        .hres = static_cast<uint32_t>(width_),
        .vres = static_cast<uint32_t>(height_),
        .monochrome = false,
        .rotation = {
            .swap_xy = swap_xy,
            .mirror_x = mirror_x,
            .mirror_y = mirror_y,
        },
        .color_format = LV_COLOR_FORMAT_RGB565,
        .flags = {
            .buff_dma = 1,
            .buff_spiram = 0,
            .sw_rotate = 0,
            .swap_bytes = 1,
            .full_refresh = 0,
            .direct_mode = 0,
        },
    };

    display_ = lvgl_port_add_disp(&display_cfg);
    if (display_ == nullptr) {
        ESP_LOGE(TAG, "Failed to add display");
        return;
    }

    if (offset_x != 0 || offset_y != 0) {
        lv_display_set_offset(display_, offset_x, offset_y);
    }

    // 初始化默认设置
    kline_frequency = 3; // 默认一小时的K线频率
    screensaver_enabled = true; // 默认启用屏保
    
    // 初始化当前显示的虚拟币数据
    current_crypto_data_.symbol = "BTC";
    current_crypto_data_.name = "Bitcoin";
    current_crypto_data_.price = 0.0;
    current_crypto_data_.change_24h = 0.0;
    current_crypto_data_.currency_id = 1; // 默认显示BTC

    // 初始化屏保虚拟币数据
    screensaver_crypto_ = current_crypto_data_;
    
    ESP_LOGI(TAG, "Initialized default settings:");
    ESP_LOGI(TAG, "Theme: %s, Crypto: %d, KLine Freq: %d", theme_name.c_str(), default_crypto, kline_frequency);

    // 初始化最后活动时间为当前时间
    last_activity_time_ = esp_timer_get_time() / 1000; // 转换为毫秒
    
    ESP_LOGI(TAG, "Last activity time initialized to: %lld", last_activity_time_);

    // 创建屏保定时器（无论是否有触摸屏都需要）
    if (screensaver_enabled) {
        esp_timer_create_args_t timer_args = {
            .callback = ScreensaverTimerCallback,
            .arg = this,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "screensaver_timer",
            .skip_unhandled_events = false,
        };
        esp_timer_create(&timer_args, &screensaver_timer_);
        ESP_LOGI(TAG, "Screensaver timer created");
    }

    // 创建虚拟币行情更新定时器
    esp_timer_create_args_t crypto_timer_args = {
        .callback = CryptoUpdateTimerCallback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "crypto_update_timer",
        .skip_unhandled_events = false,
    };
    esp_timer_create(&crypto_timer_args, &crypto_update_timer_);
    ESP_LOGI(TAG, "Crypto update timer created");

    // 初始化币界虚拟币行情数据支持
    bijie_coins_ = std::make_unique<BiJieCoins>();
    ESP_LOGI(TAG, "BiJieCoins initialized");

    // 初始化UI
    SetupUI();
    ESP_LOGI(TAG, "WXT185Display constructor completed");
}

WXT185Display::~WXT185Display() {
    ESP_LOGI(TAG, "Destroying WXT185Display instance");
    
    // 删除屏保定时器（无论是否有触摸屏都需要）
    if (screensaver_timer_) {
        esp_timer_stop(screensaver_timer_);
        esp_timer_delete(screensaver_timer_);
        ESP_LOGI(TAG, "Screensaver timer stopped and deleted");
    }
    
    // 删除虚拟币行情更新定时器
    if (crypto_update_timer_) {
        esp_timer_stop(crypto_update_timer_);
        esp_timer_delete(crypto_update_timer_);
        ESP_LOGI(TAG, "Crypto update timer stopped and deleted");
    }
    
    // 断开币界虚拟币行情数据连接
    if (bijie_coins_ && bijie_coins_connected_) {
        bijie_coins_->DisconnectAll();
        ESP_LOGI(TAG, "Disconnected all BiJie coins connections");
    }
    
    // 屏保页面对象需要手动删除，因为它直接依附于主屏幕
    if (screensaver_page_) {
        lv_obj_del(screensaver_page_);
        screensaver_page_ = nullptr;
        ESP_LOGI(TAG, "Screensaver page deleted");
    }

    if (main_screen_) {
        lv_obj_del(main_screen_);
        main_screen_ = nullptr;
        ESP_LOGI(TAG, "Main screen deleted");
    }
    
    ESP_LOGI(TAG, "WXT185Display instance destroyed");
}

void WXT185Display::SetupUI() {
   
    DisplayLockGuard lock(this);
    
    ESP_LOGI(TAG, "Setting up WXT185 UI");
    
    // 获取屏幕对象
    main_screen_ = lv_screen_active();
    lv_obj_set_style_text_font(main_screen_, fonts_.text_font, 0);
    lv_obj_set_style_text_color(main_screen_, current_wxt185_theme_.text, 0);
    lv_obj_set_style_bg_color(main_screen_, current_wxt185_theme_.background, 0);
    
    // 创建页面视图容器（针对360*360圆形屏幕优化）
    page_container_ = lv_obj_create(main_screen_);
    lv_obj_set_size(page_container_, width_, height_);
    lv_obj_set_style_radius(page_container_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(page_container_, current_wxt185_theme_.background, 0);
    lv_obj_clear_flag(page_container_, LV_OBJ_FLAG_SCROLLABLE);
    
    ESP_LOGI(TAG, "Created page view container");
    
#if CONFIG_ESP32_S3_TOUCH_LCD_185_WITH_TOUCH || CONFIG_ESP32_S3_TOUCH_LCD_185C_WITH_TOUCH
    // 添加触摸事件处理（仅在有触摸屏时添加）
    lv_obj_add_event_cb(page_container_, TouchEventHandler, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(page_container_, TouchEventHandler, LV_EVENT_RELEASED, this);
    ESP_LOGI(TAG, "Added touch event handlers to page view");
#endif
    
    // 创建表盘圆环公共组件
    //CreateCommonComponents();
    ESP_LOGI(TAG, "Created common components");

    // 创建3个页面
    CreateChatPage();
    ESP_LOGI(TAG, "Created chat page");
    
    //CreateCryptoPage();
    ESP_LOGI(TAG, "Created crypto page");
    
    //CreateSettingsPage();
    ESP_LOGI(TAG, "Created settings page");
    
    // 创建屏保页面
    CreateScreensaverPage();
    ESP_LOGI(TAG, "Created screensaver page");
    
    // 应用主题
    ApplyTheme();
    ESP_LOGI(TAG, "Applied theme");
    
    // 启动屏保定时器（无论是否有触摸屏都需要）
    StartScreensaverTimer();
    ESP_LOGI(TAG, "Started screensaver timer");
    
    // 启动虚拟币行情更新定时器
    StartCryptoUpdateTimer();
    ESP_LOGI(TAG, "Started crypto update timer");
    
    // 设置币界虚拟币行情数据回调
    if (bijie_coins_) {
        bijie_coins_->SetMarketDataCallback([this](const CoinMarketData& market_data) {
            ESP_LOGI(TAG, "Received market data callback for currency ID: %d", market_data.currency_id);
            // 更新屏保内容
            if (screensaver_active_) {
                UpdateScreensaverContent();
            }
            
            // 更新虚拟币页面内容
            UpdateCryptoData();
        });
        
        // 连接到币界虚拟币行情数据
        ConnectToBiJieCoins();
        ESP_LOGI(TAG, "Connected to BiJie coins");
    }
    
    ESP_LOGI(TAG, "WXT185 UI setup completed");
}

void WXT185Display::CreateCommonComponents()
{
    if (!main_screen_) return;
    // 1. 创建外圆环（刻度环）
    common_outer_ring_ = lv_obj_create(main_screen_);
    lv_obj_set_size(common_outer_ring_, width_ - 20, height_ - 20);
    lv_obj_center(common_outer_ring_);
    lv_obj_set_style_radius(common_outer_ring_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(common_outer_ring_, current_wxt185_theme_.outer_ring_background, 0);
    lv_obj_set_style_border_width(common_outer_ring_, 2, 0);
    lv_obj_set_style_border_color(common_outer_ring_, current_wxt185_theme_.border, 0);
    lv_obj_clear_flag(common_outer_ring_, LV_OBJ_FLAG_SCROLLABLE);
    // 设置为透明，避免挡住其他内容
    lv_obj_set_style_bg_opa(common_outer_ring_, LV_OPA_TRANSP, 0);

    // 2. 计算基础比例参数（基于屏幕直径）
    uint16_t screen_diameter = lv_obj_get_width(main_screen_);
    float scale = screen_diameter / 360.0f; // 缩放比例（以360为基准）

    // 3. 内圆环（进度环，直径为屏幕的89%）
    uint16_t progress_ring_size = screen_diameter * 0.89;
    common_inner_ring_ = lv_arc_create(main_screen_);
    lv_obj_set_size(common_inner_ring_, progress_ring_size, progress_ring_size);
    lv_obj_center(common_inner_ring_);
    lv_arc_set_angles(common_inner_ring_, 0, 270);
    // 颜色以当前虚拟币涨跌颜色为基准
    lv_obj_set_style_arc_color(common_inner_ring_,
        current_crypto_data_.change_24h >= 0 ? current_wxt185_theme_.crypto_up_color : current_wxt185_theme_.crypto_down_color, 0);
    lv_obj_set_style_arc_color(common_inner_ring_, current_wxt185_theme_.inner_ring_background, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(common_inner_ring_, scale * 4, LV_PART_INDICATOR);
    //lv_obj_set_style_arc_width(progress_ring, scale * 4, 0);
    lv_arc_set_mode(common_inner_ring_, LV_ARC_MODE_SYMMETRICAL);
    lv_obj_clear_flag(common_inner_ring_, LV_OBJ_FLAG_CLICKABLE);
    // 设置为透明，避免挡住其他内容
    lv_obj_set_style_bg_opa(common_inner_ring_, LV_OPA_TRANSP, 0);

}

void WXT185Display::CreateChatPage() {
    ESP_LOGI(TAG, "Creating chat page");
    
    chat_page_ = lv_obj_create(page_container_);
    lv_obj_set_style_bg_color(chat_page_, current_wxt185_theme_.background, 0);
    lv_obj_clear_flag(chat_page_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(chat_page_, 0, 0);
    lv_obj_set_style_border_width(chat_page_, 0, 0);
    lv_obj_set_style_bg_opa(chat_page_, LV_OPA_TRANSP, 0);
    
#if CONFIG_ESP32_S3_TOUCH_LCD_185_WITH_TOUCH || CONFIG_ESP32_S3_TOUCH_LCD_185C_WITH_TOUCH
    // 添加触摸事件处理（仅在有触摸屏时添加）
    lv_obj_add_event_cb(chat_page_, TouchEventHandler, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(chat_page_, TouchEventHandler, LV_EVENT_RELEASED, this);
    ESP_LOGV(TAG, "Added touch event handlers to chat page");
#endif
    // 先用当前的组件创建聊天页面
    /* Container */
    container_ = lv_obj_create(chat_page_);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_pad_row(container_, 0, 0);
    lv_obj_set_style_bg_color(container_, current_wxt185_theme_.background, 0);
    lv_obj_set_style_border_color(container_, current_wxt185_theme_.border, 0);

    /* Status bar */
    status_bar_ = lv_obj_create(page_container_);
    // 设置状态栏宽度为屏幕宽度的 20%
    lv_obj_set_size(status_bar_, LV_HOR_RES * 0.5f, fonts_.text_font->line_height);
    // 设置文本居中
    lv_obj_set_style_text_align(status_bar_, LV_TEXT_ALIGN_CENTER, 0);
    // 设置背景
    lv_obj_set_style_bg_color(status_bar_, current_wxt185_theme_.background, 0);
    lv_obj_set_style_text_color(status_bar_, current_wxt185_theme_.text, 0);
    // 修改对齐方式为顶部中间，并添加适当的垂直偏移量
    lv_obj_align(status_bar_, LV_ALIGN_TOP_MID, 0, 10); // 垂直偏移量为10像素
    // 设置不可滚动
    lv_obj_clear_flag(status_bar_, LV_OBJ_FLAG_SCROLLABLE);

    /* Content */
    content_ = lv_obj_create(page_container_);
    lv_obj_align(content_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_scrollbar_mode(content_, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(content_, 0, 0);
    lv_obj_set_width(content_, LV_HOR_RES);
    lv_obj_set_flex_grow(content_, 1);
    lv_obj_set_style_pad_all(content_, 5, 0);
    lv_obj_set_style_bg_color(content_, current_wxt185_theme_.chat_background, 0);
    lv_obj_set_style_border_color(content_, current_wxt185_theme_.border, 0); // Border color for content

    lv_obj_set_flex_flow(content_, LV_FLEX_FLOW_COLUMN); // 垂直布局（从上到下）
    lv_obj_set_flex_align(content_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY); // 子对象居中对齐，等距分布

    emotion_label_ = lv_label_create(content_);
    lv_obj_set_style_text_font(emotion_label_, &font_awesome_30_4, 0);
    lv_obj_set_style_text_color(emotion_label_, current_wxt185_theme_.text, 0);
    lv_label_set_text(emotion_label_, FONT_AWESOME_AI_CHIP);

    preview_image_ = lv_image_create(content_);
    lv_obj_set_size(preview_image_, width_ * 0.5, height_ * 0.5);
    lv_obj_align(preview_image_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(preview_image_, LV_OBJ_FLAG_HIDDEN);

    chat_message_label_ = lv_label_create(content_);
    lv_label_set_text(chat_message_label_, "");
    lv_obj_set_width(chat_message_label_, LV_HOR_RES * 0.9); // 限制宽度为屏幕宽度的 90%
    lv_label_set_long_mode(chat_message_label_, LV_LABEL_LONG_WRAP); // 设置为自动换行模式
    lv_obj_set_style_text_align(chat_message_label_, LV_TEXT_ALIGN_CENTER, 0); // 设置文本居中对齐
    lv_obj_set_style_text_color(chat_message_label_, current_wxt185_theme_.text, 0);

    /* Status bar */
    lv_obj_set_flex_flow(status_bar_, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(status_bar_, 0, 0);
    lv_obj_set_style_border_width(status_bar_, 0, 0);
    lv_obj_set_style_pad_column(status_bar_, 0, 0);
    lv_obj_set_style_pad_left(status_bar_, 2, 0);
    lv_obj_set_style_pad_right(status_bar_, 2, 0);

    network_label_ = lv_label_create(status_bar_);
    lv_label_set_text(network_label_, "");
    lv_obj_set_style_text_font(network_label_, fonts_.icon_font, 0);
    lv_obj_set_style_text_color(network_label_, current_wxt185_theme_.text, 0);

    notification_label_ = lv_label_create(status_bar_);
    lv_obj_set_flex_grow(notification_label_, 1);
    lv_obj_set_style_text_align(notification_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(notification_label_, current_wxt185_theme_.text, 0);
    lv_label_set_text(notification_label_, "");
    lv_obj_add_flag(notification_label_, LV_OBJ_FLAG_HIDDEN);

    status_label_ = lv_label_create(status_bar_);
    lv_obj_set_flex_grow(status_label_, 1);
    lv_label_set_long_mode(status_label_, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(status_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(status_label_, current_wxt185_theme_.text, 0);
    lv_label_set_text(status_label_, Lang::Strings::INITIALIZING);
    mute_label_ = lv_label_create(status_bar_);
    lv_label_set_text(mute_label_, "");
    lv_obj_set_style_text_font(mute_label_, fonts_.icon_font, 0);
    lv_obj_set_style_text_color(mute_label_, current_wxt185_theme_.text, 0);

    battery_label_ = lv_label_create(status_bar_);
    lv_label_set_text(battery_label_, "");
    lv_obj_set_style_text_font(battery_label_, fonts_.icon_font, 0);
    lv_obj_set_style_text_color(battery_label_, current_wxt185_theme_.text, 0);

    low_battery_popup_ = lv_obj_create(chat_page_);
    lv_obj_set_scrollbar_mode(low_battery_popup_, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_size(low_battery_popup_, LV_HOR_RES * 0.9, fonts_.text_font->line_height * 2);
    lv_obj_align(low_battery_popup_, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(low_battery_popup_, current_wxt185_theme_.low_battery, 0);
    lv_obj_set_style_radius(low_battery_popup_, 10, 0);
    low_battery_label_ = lv_label_create(low_battery_popup_);
    lv_label_set_text(low_battery_label_, Lang::Strings::BATTERY_NEED_CHARGE);
    lv_obj_set_style_text_color(low_battery_label_, lv_color_white(), 0);
    lv_obj_center(low_battery_label_);
    lv_obj_add_flag(low_battery_popup_, LV_OBJ_FLAG_HIDDEN);
    
    ESP_LOGI(TAG, "Chat page creation completed");
}

void WXT185Display::CreateCryptoPage() {
    ESP_LOGI(TAG, "Creating crypto page");
    
    // 1. 创建背景
    crypto_page_ = lv_obj_create(page_container_);
    lv_obj_set_style_bg_color(crypto_page_, current_wxt185_theme_.crypto_background, 0);
    lv_obj_clear_flag(crypto_page_, LV_OBJ_FLAG_SCROLLABLE);
    
#if CONFIG_ESP32_S3_TOUCH_LCD_185_WITH_TOUCH || CONFIG_ESP32_S3_TOUCH_LCD_185C_WITH_TOUCH
    // 添加触摸事件处理（仅在有触摸屏时添加）
    lv_obj_add_event_cb(crypto_page_, TouchEventHandler, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(crypto_page_, TouchEventHandler, LV_EVENT_RELEASED, this);
    ESP_LOGV(TAG, "Added touch event handlers to crypto page");
#endif
    // 2. 创建虚拟币roller，支持水平滚动
    crypto_roller = lv_label_create(crypto_page_);

    
    ESP_LOGI(TAG, "Crypto page creation completed");
}

void WXT185Display::CreateSettingsPage() {
    ESP_LOGI(TAG, "Creating settings page");
    
    // 1. 初始化背景
    settings_page_ = lv_obj_create(page_container_);
    ESP_LOGI(TAG, "Settings page background created");
    lv_obj_set_style_radius(settings_page_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(settings_page_, current_wxt185_theme_.background, 0);
    lv_obj_clear_flag(settings_page_, LV_OBJ_FLAG_SCROLLABLE);
    
#if CONFIG_ESP32_S3_TOUCH_LCD_185_WITH_TOUCH || CONFIG_ESP32_S3_TOUCH_LCD_185C_WITH_TOUCH
    // 添加触摸事件处理（仅在有触摸屏时添加）
    lv_obj_add_event_cb(settings_page_, TouchEventHandler, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(settings_page_, TouchEventHandler, LV_EVENT_RELEASED, this);
    ESP_LOGV(TAG, "Added touch event handlers to settings page");
#endif
    
    // 2. 创建设置标题
    settings_title_ = lv_label_create(settings_page_);
    ESP_LOGI(TAG, "Settings title label created");
    lv_label_set_text(settings_title_, "Settings");
    lv_obj_set_style_text_font(settings_title_, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(settings_title_, current_wxt185_theme_.text, 0);
    lv_obj_align(settings_title_, LV_ALIGN_TOP_MID, 0, 30);

    // 3. 创建主题设置
    settings_theme_label_ = lv_label_create(settings_page_);
    ESP_LOGI(TAG, "Theme label created");
    lv_label_set_text(settings_theme_label_, "Theme:");
    lv_obj_set_style_text_font(settings_theme_label_, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(settings_theme_label_, current_wxt185_theme_.text, 0);
    lv_obj_align(settings_theme_label_, LV_ALIGN_TOP_MID, -80, 80);

    settings_theme_roller_ = lv_roller_create(settings_page_);
    ESP_LOGI(TAG, "Theme roller created");
    lv_obj_set_style_text_font(settings_theme_roller_, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(settings_theme_roller_, current_wxt185_theme_.text, 0);
    lv_obj_set_style_bg_color(settings_theme_roller_, lv_color_hex(0x1a001a), 0);

    // 添加主题选项到roller
    char theme_options[MAX_THEME_NAME_LENGTH * ThemeCount] = {0};
    for (int i = 0; i < ThemeCount; i++) {
        strcat(theme_options, ThemeString[i]);
        if (i < ThemeCount - 1) {
            strcat(theme_options, "\n");
        }
    }
    lv_roller_set_options(settings_theme_roller_, theme_options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(settings_theme_roller_, 1);
    lv_roller_set_selected(settings_theme_roller_, selected_theme, LV_ANIM_OFF);
    lv_obj_add_event_cb(settings_theme_roller_, theme_roller_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_align_to(settings_theme_roller_, settings_theme_label_, LV_ALIGN_OUT_RIGHT_MID, 30, 0);
    lv_obj_set_width(settings_theme_roller_, 100);

    // 5. 创建默认虚拟币设置
    settings_default_crypto_label_ = lv_label_create(settings_page_);
    ESP_LOGI(TAG, "Default crypto label created");
    lv_label_set_text(settings_default_crypto_label_, "Default Coin:");
    lv_obj_set_style_text_font(settings_default_crypto_label_, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(settings_default_crypto_label_, current_wxt185_theme_.text, 0);
    lv_obj_align(settings_default_crypto_label_, LV_ALIGN_TOP_MID, -80, 120);

    settings_default_crypto_roller_ = lv_roller_create(settings_page_);
    ESP_LOGI(TAG, "Default crypto roller created");
    lv_obj_set_style_text_font(settings_default_crypto_roller_, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(settings_default_crypto_roller_, current_wxt185_theme_.text, 0);
    lv_obj_set_style_bg_color(settings_default_crypto_roller_, lv_color_hex(0x1a001a), 0);

    // 添加虚拟币选项到roller
    // 获取虚拟币列表
    int crypto_count = 0;
    if (bijie_coins_ != nullptr) {
        std::vector<CoinInfo> v = bijie_coins_->GetCoinList();
        crypto_count = v.size();
        char crypto_options[MAX_COIN_NAME_LEN * v.size()] = {0};
        for (int i = 0; i < crypto_count; i++) {
            strcat(crypto_options, v[i].name.c_str());
            if (i < crypto_count - 1) {
                strcat(crypto_options, "\n");
            }
        }
        lv_roller_set_options(settings_default_crypto_roller_, crypto_options, LV_ROLLER_MODE_NORMAL);
    }
    else {
        lv_roller_set_options(settings_default_crypto_roller_, "", LV_ROLLER_MODE_NORMAL);
    }
    lv_roller_set_visible_row_count(settings_default_crypto_roller_, 1);
    lv_roller_set_selected(settings_default_crypto_roller_, default_crypto, LV_ANIM_OFF);
    lv_obj_add_event_cb(settings_default_crypto_roller_, default_crypto_roller_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_align_to(settings_default_crypto_roller_, settings_default_crypto_label_, LV_ALIGN_OUT_RIGHT_MID, 30, 0);
    lv_obj_set_width(settings_default_crypto_roller_, 100);

    // 6. 创建K线频率设置
    settings_kline_time_label_ = lv_label_create(settings_page_);
    ESP_LOGI(TAG, "K-line time label created");
    lv_label_set_text(settings_kline_time_label_, "K-Line Freq:");
    lv_obj_set_style_text_font(settings_kline_time_label_, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(settings_kline_time_label_, current_wxt185_theme_.text, 0);
    lv_obj_align(settings_kline_time_label_, LV_ALIGN_TOP_MID, -80, 160);

    settings_kline_time_roller_ = lv_roller_create(settings_page_);
    ESP_LOGI(TAG, "K-line time roller created");
    lv_obj_set_style_text_font(settings_kline_time_roller_, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(settings_kline_time_roller_, current_wxt185_theme_.text, 0);
    lv_obj_set_style_bg_color(settings_kline_time_roller_, lv_color_hex(0x1a001a), 0);

    // 添加K线频率选项到roller
    const char** klinefreq = nullptr;
    if (bijie_coins_ != nullptr) {
        klinefreq = bijie_coins_->GetKLineTimeFrequencies();
    } else {
        static const char* default_freqs[] = {"1m", "5m", "15m", "1h", "4h", "1d", "1w", "1mo", "3mo", nullptr};
        klinefreq = default_freqs;
    }
    int kline_option_count = 0;
    const char* kline_options[10] = {};
    while (klinefreq[kline_option_count] && kline_option_count < 10) {
        kline_options[kline_option_count] = klinefreq[kline_option_count];
        kline_option_count++;
    }
    char kline_freq_options[MAX_KLINE_FREQUENCIES_LEN * kline_option_count] = {0};
    for (int i = 0; i < kline_option_count; i++) {
        strcat(kline_freq_options, kline_options[i]);
        if (i < kline_option_count - 1) {
            strcat(kline_freq_options, "\n");
        }
    }
    lv_roller_set_options(settings_kline_time_roller_, kline_freq_options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(settings_kline_time_roller_, 1);
    lv_roller_set_selected(settings_kline_time_roller_, kline_frequency, LV_ANIM_OFF);
    lv_obj_add_event_cb(settings_kline_time_roller_, kline_frequency_roller_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_align_to(settings_kline_time_roller_, settings_kline_time_label_, LV_ALIGN_OUT_RIGHT_MID, 30, 0);
    lv_obj_set_width(settings_kline_time_roller_, 100);

    // 7. 创建屏保开关
    settings_screensaver_label_ = lv_label_create(settings_page_);
    ESP_LOGI(TAG, "Screensaver label created");
    lv_label_set_text(settings_screensaver_label_, "Screensaver:");
    lv_obj_set_style_text_font(settings_screensaver_label_, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(settings_screensaver_label_, current_wxt185_theme_.text, 0);
    lv_obj_align(settings_screensaver_label_, LV_ALIGN_TOP_MID, -80, 150);

    settings_screensaver_switch_ = lv_switch_create(settings_page_);
    ESP_LOGI(TAG, "Screensaver switch created");
    lv_obj_set_style_bg_color(settings_screensaver_switch_, current_wxt185_theme_.settings_screensaver_switch, 0);
    if (screensaver_enabled) {
        lv_obj_add_state(settings_screensaver_switch_, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(settings_screensaver_switch_, screensaver_switch_event_handler, LV_EVENT_VALUE_CHANGED, this);
    lv_obj_align_to(settings_screensaver_switch_, settings_screensaver_label_, LV_ALIGN_OUT_RIGHT_MID, 55, 0);
    
    // 8. 创建保存按钮
    settings_save_button_ = lv_button_create(settings_page_);
    ESP_LOGI(TAG, "Save button created");
    lv_obj_set_style_bg_color(settings_save_button_, current_wxt185_theme_.settings_screensaver_switch, 0);
    lv_obj_set_size(settings_save_button_, 100, 40);
    lv_obj_align(settings_save_button_, LV_ALIGN_BOTTOM_MID, 0, 0);
    
    lv_obj_t* save_label = lv_label_create(settings_save_button_);
    ESP_LOGI(TAG, "Save button label created");
    lv_label_set_text(save_label, "Save");
    lv_obj_center(save_label);
    
    lv_obj_add_event_cb(settings_save_button_, settings_save_button_event_handler, LV_EVENT_CLICKED, this);
    
    ESP_LOGI(TAG, "Settings page creation completed");
}

void WXT185Display::CreateScreensaverPage() {
    // 未启动屏保时则返回
    if (!screensaver_enabled) return;
    ESP_LOGI(TAG, "Creating screensaver page");
    
    // 1. 创建背景
    screensaver_page_ = lv_obj_create(lv_scr_act());
    lv_obj_set_size(screensaver_page_, width_, height_);
    lv_obj_set_style_radius(screensaver_page_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(screensaver_page_, current_wxt185_theme_.background, 0);
    lv_obj_clear_flag(screensaver_page_, LV_OBJ_FLAG_SCROLLABLE);
    
    // 2. 创建外圆环（刻度环）
    screensaver_outer_ring_ = lv_obj_create(screensaver_page_);
    lv_obj_set_size(screensaver_outer_ring_, width_ - 20, width_ - 20);
    lv_obj_center(screensaver_outer_ring_);
    lv_obj_set_style_radius(screensaver_outer_ring_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(screensaver_outer_ring_, current_wxt185_theme_.outer_ring_background, 0);
    lv_obj_set_style_border_width(screensaver_outer_ring_, 2, 0);
    lv_obj_set_style_border_color(screensaver_outer_ring_, current_wxt185_theme_.border, 0);
    lv_obj_clear_flag(screensaver_outer_ring_, LV_OBJ_FLAG_SCROLLABLE);

    // 3. 计算基础比例参数（基于屏幕直径）
    uint16_t screen_diameter = lv_obj_get_width(screensaver_outer_ring_);
    float scale = screen_diameter / 360.0f; // 缩放比例（以360为基准）

    // 4. 内圆环（进度环，直径为屏幕的89%）
    uint16_t progress_ring_size = screen_diameter * 0.89;
    screensaver_progress_ring_ = lv_arc_create(screensaver_page_);
    lv_obj_set_size(screensaver_progress_ring_, progress_ring_size, progress_ring_size);
    lv_obj_center(screensaver_progress_ring_);
    lv_arc_set_angles(screensaver_progress_ring_, 0, 270);
    lv_obj_set_style_arc_color(screensaver_progress_ring_,
        screensaver_crypto_.change_24h >= 0 ? current_wxt185_theme_.crypto_up_color : current_wxt185_theme_.crypto_down_color, 0);
    lv_obj_set_style_arc_color(screensaver_progress_ring_, current_wxt185_theme_.crypto_progress_bg_color, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(screensaver_progress_ring_, scale * 4, LV_PART_INDICATOR);
    //lv_obj_set_style_arc_width(screensaver_progress_ring_, scale * 4, 0);
    lv_arc_set_mode(screensaver_progress_ring_, LV_ARC_MODE_SYMMETRICAL);
    lv_obj_clear_flag(screensaver_progress_ring_, LV_OBJ_FLAG_CLICKABLE);

    // 5. 创建币名显示
    screensaver_crypto_name_ = lv_label_create(screensaver_page_);
    lv_label_set_text_fmt(screensaver_crypto_name_, "%s", screensaver_crypto_.symbol.c_str());
    lv_obj_set_style_text_font(screensaver_crypto_name_, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(screensaver_crypto_name_, current_wxt185_theme_.text, 0);
    lv_obj_align(screensaver_crypto_name_, LV_ALIGN_CENTER, 0, -60);

    // 6. 创建全称显示
    screensaver_crypto_fullname = lv_label_create(screensaver_page_);
    lv_label_set_text_fmt(screensaver_crypto_fullname, "%s", screensaver_crypto_.name.c_str());
    lv_obj_set_style_text_font(screensaver_crypto_fullname, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(screensaver_crypto_fullname, current_wxt185_theme_.crypto_sub_text, 0);
    lv_obj_align_to(screensaver_crypto_fullname, screensaver_crypto_name_, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    // 7. 创建价格显示
    screensaver_crypto_price_ = lv_label_create(screensaver_page_);
    lv_label_set_text_fmt(screensaver_crypto_price_, "$%.2f", screensaver_crypto_.price);
    lv_obj_set_style_text_font(screensaver_crypto_price_, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(screensaver_crypto_price_, current_wxt185_theme_.text, 0);
    lv_obj_align(screensaver_crypto_price_, LV_ALIGN_CENTER, 0, 20);

    // 8. 创建涨跌幅显示
    screensaver_crypto_change_ = lv_label_create(screensaver_page_);
    lv_label_set_text_fmt(screensaver_crypto_change_, "%.2f%%", screensaver_crypto_.change_24h);
    lv_obj_set_style_text_font(screensaver_crypto_change_, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(screensaver_crypto_change_,
        screensaver_crypto_.change_24h >= 0 ? current_wxt185_theme_.crypto_up_color : current_wxt185_theme_.crypto_down_color, 0);
    lv_obj_align_to(screensaver_crypto_change_, screensaver_crypto_price_, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    // 9. 创建时间显示标签
    screensaver_time_ = lv_label_create(screensaver_page_);
    lv_label_set_text(screensaver_time_, "");
    lv_obj_set_style_text_font(screensaver_time_, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(screensaver_time_, current_wxt185_theme_.crypto_sub_text, 0);
    lv_obj_align(screensaver_time_, LV_ALIGN_BOTTOM_MID, 0, -60);

    // 初始隐藏屏保页面
    lv_obj_add_flag(screensaver_page_, LV_OBJ_FLAG_HIDDEN);

    
    ESP_LOGI(TAG, "Screensaver page creation completed");
}

void WXT185Display::ApplyTheme() {
    ESP_LOGI(TAG, "Applying theme");
    
    DisplayLockGuard lock(this);
    
    // 应用主屏幕背景色
    lv_obj_set_style_bg_color(main_screen_, current_wxt185_theme_.background, 0);
    
    ApplyChatPageTheme();
    ESP_LOGV(TAG, "Applied chat page theme");
    
    ApplyCryptoPageTheme();
    ESP_LOGV(TAG, "Applied crypto page theme");
    
    ApplySettingsPageTheme();
    ESP_LOGV(TAG, "Applied settings page theme");
    
    ApplyScreensaverTheme(); // 应用屏保主题
    ESP_LOGV(TAG, "Applied screensaver theme");
    
    ESP_LOGI(TAG, "Theme applied successfully");
}

void WXT185Display::ApplyChatPageTheme() {
    if (!chat_page_ || !chat_status_bar_ || !chat_content_) return;
    ESP_LOGI(TAG, "Applying chat page theme");
    
    // 应用聊天页面主题
    lv_obj_set_style_bg_color(chat_page_, current_wxt185_theme_.background, 0);
    lv_obj_set_style_bg_opa(chat_page_, LV_OPA_COVER, 0);
    
    // 应用状态栏主题
    lv_obj_set_style_bg_color(chat_status_bar_, current_wxt185_theme_.header, 0);
    lv_obj_set_style_text_color(chat_status_bar_, current_wxt185_theme_.text, 0);
    
    // 应用内容区域主题
    lv_obj_set_style_bg_color(chat_content_, current_wxt185_theme_.chat_background, 0);
    lv_obj_set_style_border_color(chat_content_, current_wxt185_theme_.border, 0);
}

void WXT185Display::ApplyCryptoPageTheme() {
    if (!crypto_page_ || !crypto_header_ || !crypto_chart_ || !crypto_list_) return;
    ESP_LOGI(TAG, "Applying crypto page theme");
    
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
    if (!settings_page_ || !settings_title_ || !settings_theme_roller_ || 
        !settings_default_crypto_roller_ || !settings_kline_time_roller_) return;
    ESP_LOGI(TAG, "Applying settings page theme");
    
    // 应用设置页面主题
    lv_obj_set_style_bg_color(settings_page_, current_wxt185_theme_.background, 0);
    
    // 应用头部区域主题
    lv_obj_set_style_text_color(settings_title_, current_wxt185_theme_.text, 0);
    
    // 应用主题选择区域主题
    lv_obj_set_style_bg_color(settings_theme_roller_, current_wxt185_theme_.selector, 0);
    lv_obj_set_style_border_color(settings_theme_roller_, current_wxt185_theme_.border, 0);
    
    // 应用虚拟币选择区域主题
    lv_obj_set_style_bg_color(settings_default_crypto_roller_, current_wxt185_theme_.selector, 0);
    lv_obj_set_style_border_color(settings_default_crypto_roller_, current_wxt185_theme_.border, 0);
    
    // 应用时间框架选择区域主题
    lv_obj_set_style_bg_color(settings_kline_time_roller_, current_wxt185_theme_.selector, 0);
    lv_obj_set_style_border_color(settings_kline_time_roller_, current_wxt185_theme_.border, 0);
}

void WXT185Display::ApplyScreensaverTheme() {
    if (!screensaver_page_ || !screensaver_outer_ring_) return;

    ESP_LOGI(TAG, "Applying screensaver theme");
    
    // 应用屏保页面主题
    lv_obj_set_style_bg_color(screensaver_page_, current_wxt185_theme_.background, 0);
    
    // 应用屏保容器主题
    lv_obj_set_style_bg_color(screensaver_outer_ring_, current_wxt185_theme_.header, 0);
    lv_obj_set_style_bg_opa(screensaver_outer_ring_, LV_OPA_90, 0);
    
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
        lv_obj_set_style_text_color(screensaver_time_, current_wxt185_theme_.crypto_sub_text, 0);
    }
}

void WXT185Display::SetEmotion(const char* emotion) {
    ESP_LOGI(TAG, "Setting emotion: %s", emotion ? emotion : "null");
    LcdDisplay::SetEmotion(emotion);
}

void WXT185Display::SetIcon(const char* icon) {
    ESP_LOGI(TAG, "Setting icon: %s", icon ? icon : "null");
    LcdDisplay::SetIcon(icon);
}

void WXT185Display::SetPreviewImage(const lv_img_dsc_t* img_dsc) {
    ESP_LOGI(TAG, "Setting preview image");
    LcdDisplay::SetPreviewImage(img_dsc);
}

void WXT185Display::SetChatMessage(const char* role, const char* content) {
    ESP_LOGI(TAG, "Setting chat message - Role: %s, Content: %.50s...", role, content);
    // 用户活动时更新活动时间
    OnActivity();
    
    DisplayLockGuard lock(this);
    if (content_ == nullptr) return;
    
    // 确保聊天页面可见
    if (screensaver_active_) {
        ExitScreensaver();
    }
    
    // 直接更新已创建的消息标签而不是清理整个容器
    if (chat_message_label_) {
        lv_label_set_text(chat_message_label_, content);
        
        // 根据消息角色设置样式
        if (strcmp(role, "user") == 0) {
            lv_obj_set_style_text_color(chat_message_label_, current_wxt185_theme_.user_bubble, 0);
            ESP_LOGI(TAG, "Set message color to user bubble color");
        } else if (strcmp(role, "assistant") == 0) {
            lv_obj_set_style_text_color(chat_message_label_, current_wxt185_theme_.assistant_bubble, 0);
            ESP_LOGI(TAG, "Set message color to assistant bubble color");
        } else if (strcmp(role, "system") == 0) {
            lv_obj_set_style_text_color(chat_message_label_, current_wxt185_theme_.system_text, 0);
            ESP_LOGI(TAG, "Set message color to system text color");
        } else {
            // 默认文本颜色
            lv_obj_set_style_text_color(chat_message_label_, current_wxt185_theme_.text, 0);
        }
    }
}

void WXT185Display::SetTheme(const std::string& theme_name) {
    ESP_LOGI(TAG, "Setting theme: %s", theme_name.c_str());
    DisplayLockGuard lock(this);
    // 然后处理自定义主题
    if (theme_name == "dark" || theme_name == "DARK") {
        current_wxt185_theme_ = DARK_THEME_WXT185;
        ESP_LOGI(TAG, "Applied DARK theme");
    } else if (theme_name == "light" || theme_name == "LIGHT") {
        current_wxt185_theme_ = LIGHT_THEME_WXT185;
        ESP_LOGI(TAG, "Applied LIGHT theme");
    } else if (theme_name == "metal") {
        current_wxt185_theme_ = METAL_THEME_WXT185;
        ESP_LOGI(TAG, "Applied METAL theme");
    } else if (theme_name == "technology") {
        current_wxt185_theme_ = TECHNOLOGY_THEME_WXT185;
        ESP_LOGI(TAG, "Applied TECHNOLOGY theme");
    } else if (theme_name == "cosmic") {
        current_wxt185_theme_ = COSMIC_THEME_WXT185;
        ESP_LOGI(TAG, "Applied COSMIC theme");
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
    ESP_LOGI(TAG, "Drawing K-line chart");
    DisplayLockGuard lock(this);
    if (crypto_chart_ == nullptr) return;
    
    try {
        // 清除现有图表
        lv_obj_clean(crypto_chart_);
        
        // 检查币界服务是否已初始化
        if (!bijie_coins_) {
            ESP_LOGW(TAG, "BiJie coins service not initialized");
            return;
        }
        
        // 获取当前货币的K线数据
        auto market_data = bijie_coins_->GetMarketData(current_crypto_data_.currency_id);
        if (!market_data) {
            ESP_LOGW(TAG, "No market data available for currency ID: %d", current_crypto_data_.currency_id);
            return;
        }
        
        // 检查是否有K线数据
        if (market_data->kline_data_1h.empty()) {
            ESP_LOGW(TAG, "No K-line data available for currency ID: %d", current_crypto_data_.currency_id);
            return;
        }
        
        // 创建图表对象
        lv_obj_t* chart = lv_chart_create(crypto_chart_);
        if (!chart) {
            ESP_LOGE(TAG, "Failed to create chart object");
            return;
        }
        
        lv_obj_set_size(chart, lv_obj_get_width(crypto_chart_) - 20, lv_obj_get_height(crypto_chart_) - 20);
        lv_obj_center(chart);
        
        // 设置图表样式
        lv_chart_set_type(chart, LV_CHART_TYPE_LINE); // 使用线图替代K线图，因为LVGL不直接支持K线图
        lv_chart_set_div_line_count(chart, 5, 5);
        
        // 设置图表样式
        lv_obj_set_style_bg_color(chart, current_wxt185_theme_.background, 0);
        lv_obj_set_style_border_color(chart, current_wxt185_theme_.border, 0);
        lv_obj_set_style_text_color(chart, current_wxt185_theme_.text, 0);
        
        // 添加数据系列 - 只显示收盘价
        lv_chart_series_t* close_ser = lv_chart_add_series(chart, lv_color_hex(0x00FF00), LV_CHART_AXIS_PRIMARY_Y);
        if (!close_ser) {
            ESP_LOGE(TAG, "Failed to add chart series");
            return;
        }
        
        // 获取K线数据（使用1小时K线作为示例）
        const auto& kline_data = market_data->kline_data_1h;
        
        // 添加点到图表
        int point_count = 0;
        for (size_t i = 0; i < kline_data.size() && point_count < 30; i++) { // 限制显示30个点
            // 只添加收盘价到图表
            lv_chart_set_next_value(chart, close_ser, static_cast<int32_t>(kline_data[i].second * 100)); // 收盘价
            point_count++;
        }
        
        ESP_LOGI(TAG, "Added %d points to K-line chart", point_count);
        
        // 添加标题
        lv_obj_t* chart_title = lv_label_create(crypto_chart_);
        if (chart_title) {
            lv_label_set_text(chart_title, "Price Trend (Close Prices)");
            lv_obj_set_style_text_color(chart_title, current_wxt185_theme_.text, 0);
            lv_obj_align_to(chart_title, chart, LV_ALIGN_OUT_TOP_MID, 0, -5);
        }
    } catch (const std::exception& e) {
        ESP_LOGE(TAG, "Exception occurred while drawing K-line chart: %s", e.what());
    } catch (...) {
        ESP_LOGE(TAG, "Unknown exception occurred while drawing K-line chart");
    }
}

void WXT185Display::TouchEventHandler(lv_event_t* e) {
#if CONFIG_ESP32_S3_TOUCH_LCD_185_WITH_TOUCH || CONFIG_ESP32_S3_TOUCH_LCD_185C_WITH_TOUCH
    WXT185Display* self = static_cast<WXT185Display*>(lv_event_get_user_data(e));
    lv_event_code_t code = lv_event_get_code(e);
    
    ESP_LOGI(TAG, "Touch event handler called, code: %d", code);
    
    // 触摸事件视为用户活动
    self->OnActivity();
    
    if (code == LV_EVENT_PRESSED) {
        // 记录触摸开始点
        lv_indev_t* indev = lv_indev_get_act();
        if (indev) {
            lv_point_t point;
            lv_indev_get_point(indev, &point);
            ESP_LOGI(TAG, "Touch pressed at (%d, %d)", point.x, point.y);
            self->HandleTouchStart(point);
        }
    } else if (code == LV_EVENT_RELEASED) {
        // 处理触摸释放
        lv_indev_t* indev = lv_indev_get_act();
        if (indev) {
            lv_point_t point;
            lv_indev_get_point(indev, &point);
            ESP_LOGI(TAG, "Touch released at (%d, %d)", point.x, point.y);
            self->HandleTouchEnd(point);
        }
    }
#endif
}

void WXT185Display::HandleTouchStart(lv_point_t point) {
    ESP_LOGI(TAG, "Handling touch start at (%d, %d)", point.x, point.y);
    touch_start_point_ = point;
    is_touching_ = true;
}

void WXT185Display::HandleTouchEnd(lv_point_t point) {
    ESP_LOGI(TAG, "Handling touch end at (%d, %d)", point.x, point.y);
    if (!is_touching_) return;
    
    is_touching_ = false;
    
    // 计算滑动距离
    int16_t diff_x = point.x - touch_start_point_.x;
    int16_t diff_y = point.y - touch_start_point_.y;
    
    // 判断是否为水平滑动且距离足够
    if (abs(diff_x) > abs(diff_y) && abs(diff_x) > 30) {
        if (diff_x > 0) {
            // 向右滑动，切换到上一个页面
            ESP_LOGI(TAG, "Swipe right detected, switching to previous page");
            if (current_page_index_ > 0) {
                SwitchToPage(current_page_index_ - 1);
            }
        } else {
            // 向左滑动，切换到下一个页面
            ESP_LOGI(TAG, "Swipe left detected, switching to next page");
            if (current_page_index_ < 2) {
                SwitchToPage(current_page_index_ + 1);
            }
        }
    }
}

void WXT185Display::SwitchToPage(int page_index) {
    ESP_LOGI(TAG, "Switching to page %d", page_index);
    DisplayLockGuard lock(this);
    if (page_container_ == nullptr || page_index < 0 || page_index > 2) return;
    
    current_page_index_ = page_index;
    
    // 滚动到指定页面
    lv_obj_t* target_page = nullptr;
    switch (page_index) {
        case 0:
            target_page = chat_page_;
            ESP_LOGI(TAG, "Switching to chat page");
            break;
        case 1:
            target_page = crypto_page_;
            ESP_LOGI(TAG, "Switching to crypto page");
            break;
        case 2:
            target_page = settings_page_;
            ESP_LOGI(TAG, "Switching to settings page");
            break;
        default:
            return;
    }
    
    if (target_page) {
        lv_obj_scroll_to_view_recursive(target_page, LV_ANIM_ON);
    }
}

void WXT185Display::PageEventHandler(lv_event_t* e) {
    ESP_LOGI(TAG, "Page event handler called");
    // 页面事件处理
}

void WXT185Display::CryptoSelectorEventHandler(lv_event_t* e) {
    ESP_LOGI(TAG, "Crypto selector event handler called");
    // 虚拟币选择事件处理
}

void WXT185Display::ThemeSelectorEventHandler(lv_event_t* e) {
    ESP_LOGI(TAG, "Theme selector event handler called");
    // 主题选择事件处理
}

void WXT185Display::TimeframeSelectorEventHandler(lv_event_t* e) {
    ESP_LOGI(TAG, "Timeframe selector event handler called");
    // 时间框架选择事件处理
}

void WXT185Display::ScreensaverCryptoSelectorEventHandler(lv_event_t* e) {
    ESP_LOGI(TAG, "Screensaver crypto selector event handler called");
    // 屏保虚拟币选择事件处理
}

void WXT185Display::SettingsSaveButtonEventHandler(lv_event_t* e) {
    ESP_LOGI(TAG, "Settings save button event handler called");
    // 保存按钮事件处理已经在静态函数中实现
}

void WXT185Display::ScreensaverTimerCallback(void* arg) {
    try {
        WXT185Display* self = static_cast<WXT185Display*>(arg);
        if (!self) {
            ESP_LOGE(TAG, "ScreensaverTimerCallback: self is null");
            return;
        }
        
        ESP_LOGI(TAG, "Screensaver timer callback triggered");
        
        // 检查屏保功能是否启用
        extern bool screensaver_enabled;
        if (!screensaver_enabled) {
            ESP_LOGI(TAG, "Screensaver is disabled, exiting callback");
            return;
        }
        
        // 检查设备当前状态，避免在配网模式下进入屏保
        // 添加异常处理，防止Application实例访问失败
        try {
            auto& app = Application::GetInstance();
            DeviceState current_state = app.GetDeviceState();
            if (current_state == kDeviceStateWifiConfiguring) {
                ESP_LOGI(TAG, "Device is in WiFi configuring mode, skip screensaver");
                return;
            }
            if (current_state == kDeviceStateStarting) {
                ESP_LOGI(TAG, "Device is in starting mode, skip screensaver");
                return;
            }
            if (current_state == kDeviceStateActivating) {
                ESP_LOGI(TAG, "Device is in activating mode, skip screensaver");
                return;

            }
            if (current_state == kDeviceStateConnecting) {
                ESP_LOGI(TAG, "Device is in connecting mode, skip screensaver");
                return;
            }
            if (current_state == kDeviceStateFatalError) {
                ESP_LOGI(TAG, "Device is in offline mode, skip screensaver");
                return;
            }
        } catch (const std::exception& e) {
            ESP_LOGW(TAG, "Exception while getting device state: %s", e.what());
        } catch (...) {
            ESP_LOGW(TAG, "Unknown exception while getting device state");
        }

        // 检查是否超时
        int64_t current_time = esp_timer_get_time() / 1000; // 转换为毫秒
        if (current_time - self->last_activity_time_ >= SCREENSAVER_TIMEOUT_MS) {
            ESP_LOGI(TAG, "Screensaver timeout reached, entering screensaver mode");
            // 进入屏保模式
            self->EnterScreensaver();
        } else {
            ESP_LOGV(TAG, "Screensaver timeout not reached yet");
        }
    } catch (const std::exception& e) {
        ESP_LOGE(TAG, "Exception occurred in ScreensaverTimerCallback: %s", e.what());
    } catch (...) {
        ESP_LOGE(TAG, "Unknown exception occurred in ScreensaverTimerCallback");
    }
}

void WXT185Display::StartScreensaverTimer() {
    if (screensaver_timer_) {
        esp_timer_stop(screensaver_timer_);
        esp_timer_start_periodic(screensaver_timer_, 1000000); // 每秒检查一次
    }
}

void WXT185Display::CryptoUpdateTimerCallback(void* arg) {
    WXT185Display* self = static_cast<WXT185Display*>(arg);
    
    try {
        // 检查是否启用了币界虚拟币行情数据或屏保功能
        if ((self->bijie_coins_ && self->bijie_coins_connected_) || screensaver_enabled) {
            ESP_LOGI(TAG, "Crypto update timer triggered");
            
            // 使用LVGL异步调用来更新UI，确保在LVGL线程中执行
            lv_async_call([](void* user_data) {
                try {
                    WXT185Display* self = static_cast<WXT185Display*>(user_data);

                    // 触发Conenct
                    if (!self->bijie_coins_connected_) {
                        self->ConnectToBiJieCoins();
                    }
                    
                    // 更新虚拟币数据
                    self->UpdateCryptoDataFromBiJie();
                    
                    // 如果屏保处于激活状态，更新屏保内容
                    if (self->screensaver_active_) {
                        self->UpdateScreensaverContent();
                    }
                    
                    // 如果当前在虚拟币页面，更新虚拟币页面内容
                    if (self->current_page_index_ == 1) {  // 1是虚拟币页面索引
                        self->UpdateCryptoData();
                    }
                } catch (const std::exception& e) {
                    ESP_LOGE(TAG, "Exception occurred in CryptoUpdateTimerCallback: %s", e.what());
                } catch (...) {
                    ESP_LOGE(TAG, "Unknown exception occurred in CryptoUpdateTimerCallback");
                }
            }, self);
        }
    } catch (const std::exception& e) {
        ESP_LOGE(TAG, "Exception occurred in CryptoUpdateTimerCallback: %s", e.what());
    } catch (...) {
        ESP_LOGE(TAG, "Unknown exception occurred in CryptoUpdateTimerCallback");
    }
}

                    }
                } catch (const std::exception& e) {
                    ESP_LOGE(TAG, "Exception occurred in LVGL async call: %s", e.what());
                } catch (...) {
                    ESP_LOGE(TAG, "Unknown exception occurred in LVGL async call");
                }
            }, self);
        }
    } catch (const std::exception& e) {
        ESP_LOGE(TAG, "Exception occurred in CryptoUpdateTimerCallback: %s", e.what());
    } catch (...) {
        ESP_LOGE(TAG, "Unknown exception occurred in CryptoUpdateTimerCallback");
    }
    
    // 重新启动定时器，每30秒更新一次行情
    if (self->crypto_update_timer_) {
        esp_timer_start_once(self->crypto_update_timer_, 30 * 1000 * 1000); // 30秒
    }
}

void WXT185Display::StartCryptoUpdateTimer() {
    if (crypto_update_timer_) {
        esp_timer_stop(crypto_update_timer_);
        // 首次启动设置为5秒后执行，给连接一些初始化时间
        esp_timer_start_once(crypto_update_timer_, 5 * 1000 * 1000);
        ESP_LOGI(TAG, "Crypto update timer started");
    }
}

void WXT185Display::StopCryptoUpdateTimer() {
    if (crypto_update_timer_) {
        esp_timer_stop(crypto_update_timer_);
        ESP_LOGI(TAG, "Crypto update timer stopped");
    }
}

void WXT185Display::StopScreensaverTimer() {
    if (screensaver_timer_) {
        esp_timer_stop(screensaver_timer_);
    }
}

void WXT185Display::EnterScreensaver() {
    try {
        DisplayLockGuard lock(this);
        
        if (screensaver_active_ || !screensaver_page_) return;
        
        screensaver_active_ = true;
        ESP_LOGI(TAG, "Entering screensaver mode begin");
        
        // 隐藏当前页面
        if (chat_page_) lv_obj_add_flag(chat_page_, LV_OBJ_FLAG_HIDDEN);
        if (crypto_page_) lv_obj_add_flag(crypto_page_, LV_OBJ_FLAG_HIDDEN);
        if (settings_page_) lv_obj_add_flag(settings_page_, LV_OBJ_FLAG_HIDDEN);
        
        // 显示屏保页面
        if (screensaver_page_) lv_obj_clear_flag(screensaver_page_, LV_OBJ_FLAG_HIDDEN);
        
        // 更新屏保内容
        UpdateScreensaverContent();
        
        ESP_LOGI(TAG, "Entered screensaver mode end");
    } catch (const std::exception& e) {
        ESP_LOGE(TAG, "Exception occurred while entering screensaver: %s", e.what());
        screensaver_active_ = false; // 确保状态一致性
    } catch (...) {
        ESP_LOGE(TAG, "Unknown exception occurred while entering screensaver");
        screensaver_active_ = false; // 确保状态一致性
    }
}

void WXT185Display::ExitScreensaver() {
    try {
        DisplayLockGuard lock(this);
        
        if (!screensaver_active_ || !screensaver_page_) return;
        
        screensaver_active_ = false;
        
        // 隐藏屏保页面
        lv_obj_add_flag(screensaver_page_, LV_OBJ_FLAG_HIDDEN);
        
        // 显示当前页面
        switch (current_page_index_) {
            case 0:
                if (chat_page_) lv_obj_clear_flag(chat_page_, LV_OBJ_FLAG_HIDDEN);
                break;
            case 1:
                if (crypto_page_) lv_obj_clear_flag(crypto_page_, LV_OBJ_FLAG_HIDDEN);
                break;
            case 2:
                if (settings_page_) lv_obj_clear_flag(settings_page_, LV_OBJ_FLAG_HIDDEN);
                break;
        }
        
        ESP_LOGI(TAG, "Exited screensaver mode");
    } catch (const std::exception& e) {
        ESP_LOGE(TAG, "Exception occurred while exiting screensaver: %s", e.what());
    } catch (...) {
        ESP_LOGE(TAG, "Unknown exception occurred while exiting screensaver");
    }
}

void WXT185Display::UpdateScreensaverContent() {
    DisplayLockGuard lock(this);
    
    if (!screensaver_active_) return;

    ESP_LOGI(TAG, "Updating screensaver content");
    
    // 检查屏保相关对象是否存在
    if (!screensaver_page_) {
        ESP_LOGW(TAG, "Screensaver page is null");
        return;
    }

    // 更新虚拟币名称
    if (screensaver_crypto_name_) {
        lv_label_set_text(screensaver_crypto_name_, screensaver_crypto_.name.c_str());
    }
    
    // 检查币界服务是否已初始化
    if (bijie_coins_) {
        try {
            // 从币界获取屏保虚拟币数据
            auto market_data = bijie_coins_->GetMarketData(screensaver_crypto_.currency_id);
            
            if (!market_data) {
                ESP_LOGW(TAG, "No market data available for currency ID: %d", screensaver_crypto_.currency_id);
                return;
            }
            
            // 更新价格
            if (screensaver_crypto_price_) {
                char price_text[32];
                snprintf(price_text, sizeof(price_text), "$%.2f", market_data->close);
                lv_label_set_text(screensaver_crypto_price_, price_text);
            }
            
            // 更新涨跌幅
            if (screensaver_crypto_change_) {
                char change_text[32];
                snprintf(change_text, sizeof(change_text), "%.2f%%", market_data->change_24h);
                lv_label_set_text(screensaver_crypto_change_, change_text);
                
                // 根据涨跌设置颜色
                if (market_data->change_24h >= 0) {
                    lv_obj_set_style_text_color(screensaver_crypto_change_, lv_color_hex(0x00FF00), 0); // 绿色
                } else {
                    lv_obj_set_style_text_color(screensaver_crypto_change_, lv_color_hex(0xFF0000), 0); // 红色
                }
            }
        } catch (const std::exception& e) {
            ESP_LOGE(TAG, "Exception occurred while updating screensaver market data: %s", e.what());
        } catch (...) {
            ESP_LOGE(TAG, "Unknown exception occurred while updating screensaver market data");
        }
    } else {
        ESP_LOGW(TAG, "BiJie coins service not initialized");
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

    ESP_LOGI(TAG, "Screensaver content updated, getting KLine data...");
    
    // 获取K线数据用于屏保显示
    if (bijie_coins_) {
        try {
            bijie_coins_->GetKLineData(screensaver_crypto_.currency_id, 2, 30, [this](const std::vector<KLineData>& kline_data) {
                ESP_LOGI(TAG, "Received K-line data for screensaver with %d points", (int)kline_data.size());
                // 这里可以更新屏保的K线图表，但由于屏保页面结构限制，暂不实现
                // 在完整实现中，可以在这里更新屏保的K线图表显示
            });
        } catch (const std::exception& e) {
            ESP_LOGE(TAG, "Exception occurred while getting K-line data: %s", e.what());
        } catch (...) {
            ESP_LOGE(TAG, "Unknown exception occurred while getting K-line data");
        }
    }
}

void WXT185Display::OnActivity() {
    ESP_LOGI(TAG, "User activity detected, updating last activity time");
    // 更新最后活动时间
    last_activity_time_ = esp_timer_get_time() / 1000; // 转换为毫秒

    // 如果当前处于屏保状态，则退出屏保
    if (screensaver_active_) {
        ESP_LOGI(TAG, "Currently in screensaver mode, exiting due to user activity");
        ExitScreensaver();
    }
}

void WXT185Display::OnConversationStart() {
    ESP_LOGI(TAG, "Conversation started, treating as user activity");
    // 对话开始时视为用户活动
    OnActivity();
}

void WXT185Display::OnConversationEnd() {
    ESP_LOGI(TAG, "Conversation ended, updating activity time");
    // 对话结束时更新活动时间，10秒后可能进入屏保
    last_activity_time_ = esp_timer_get_time() / 1000; // 转换为毫秒
}

void WXT185Display::OnIdle() {
    ESP_LOGI(TAG, "Device idle, updating activity time for screensaver");
    // 空闲状态时更新活动时间，10秒后可能进入屏保
    last_activity_time_ = esp_timer_get_time() / 1000; // 转换为毫秒
}

void WXT185Display::OnDeviceStateChanged(int previous_state, int current_state) {
    ESP_LOGI(TAG, "Device state changed from %d to %d", previous_state, current_state);
    // 根据设备状态变化控制屏保
    switch (current_state) {
        case kDeviceStateIdle:
            // 设备进入空闲状态，设置屏保计时器
            ESP_LOGI(TAG, "Device entered idle state");
            OnIdle();
            break;
            
        case kDeviceStateListening:
        case kDeviceStateSpeaking:
            // 设备开始对话，视为用户活动
            ESP_LOGI(TAG, "Device started conversation");
            OnConversationStart();
            break;

        case kDeviceStateConnecting:
            // 设备连接状态变化也视为用户活动
            ESP_LOGI(TAG, "Device connecting");
            OnActivity();
            break;
        case kDeviceStateWifiConfiguring:
            // 设备进入WiFi配置状态也视为用户活动
            ESP_LOGI(TAG, "Device configuring WiFi");
            OnActivity();
            break;
        default:
            // 其他状态变化也视为用户活动
            ESP_LOGI(TAG, "Other device state change, treating as user activity");
            OnActivity();
            break;
    }
}

bool WXT185Display::WaitForNetworkReady(int max_wait_time) {
    auto& app = Application::GetInstance();
    DeviceState current_state = app.GetDeviceState();
    // 检查设备状态是否已经联网就绪
    ESP_LOGI(TAG, "Checking device state for network ready");
    // 循环检查设备状态
    // 计算最大重试次数
    int retry_count = 0;
    const int retry_interval = 1000; // 1秒间隔
    const int max_retries = max_wait_time / retry_interval;
    
    while (retry_count < max_retries) {
        // 检查设备状态是否已经联网就绪
        current_state = app.GetDeviceState();
        if (current_state == kDeviceStateIdle || 
            current_state == kDeviceStateListening ||
            current_state == kDeviceStateSpeaking) {
            // 设备已经就绪
            ESP_LOGI(TAG, "Network is ready, current state: %d", current_state);
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(retry_interval));
        retry_count++;
    }

    ESP_LOGE(TAG, "Network is not ready after waiting");
    return false;
}

void WXT185Display::ConnectToBiJieCoins() {
    ESP_LOGI(TAG, "Connecting to BiJie coins service");
    if (!bijie_coins_) {
        ESP_LOGW(TAG, "BiJie coins service not initialized");
        return;
    }
    
    // 创建一个专门的任务来处理WebSocket连接，避免阻塞UI线程
    // 传递this指针作为参数，以便在任务中访问WXT185Display对象
    xTaskCreate([](void* param) {
        WXT185Display* self = static_cast<WXT185Display*>(param);
        
        try {
            // 等待网络就绪
            if (!self->WaitForNetworkReady()) {
                ESP_LOGE(TAG, "Network is not ready, aborting BiJie coins connection");
                vTaskDelete(nullptr);
                return;
            }
            
            // 连接到当前显示的虚拟币行情数据
            if (self->bijie_coins_->Connect(self->current_crypto_data_.currency_id)) {
                ESP_LOGI(TAG, "Connected to BiJie coins WebSocket for currency %d", self->current_crypto_data_.currency_id);
            } else {
                ESP_LOGE(TAG, "Failed to connect to BiJie coins WebSocket for currency %d", self->current_crypto_data_.currency_id);
            }
            
            // 连接到屏保显示的虚拟币行情数据（如果不同的话）
            if (self->screensaver_crypto_.currency_id != self->current_crypto_data_.currency_id) {
                if (self->bijie_coins_->Connect(self->screensaver_crypto_.currency_id)) {
                    ESP_LOGI(TAG, "Connected to BiJie coins WebSocket for screensaver currency %d", self->screensaver_crypto_.currency_id);
                } else {
                    ESP_LOGE(TAG, "Failed to connect to BiJie coins WebSocket for screensaver currency %d", self->screensaver_crypto_.currency_id);
                }
            }
            
            // 获取K线数据用于图表显示
            self->bijie_coins_->GetKLineData(self->current_crypto_data_.currency_id, 2, 30, [self](const std::vector<KLineData>& kline_data) {
                try {
                    // 使用LVGL异步调用更新UI，确保在LVGL线程中执行
                    lv_async_call([](void* user_data) {
                        auto kline_data_ptr = static_cast<std::pair<WXT185Display*, std::vector<KLineData>*>*>(user_data);
                        WXT185Display* self = kline_data_ptr->first;
                        std::vector<KLineData>* kline_data = kline_data_ptr->second;
                        
                        ESP_LOGI(TAG, "Received K-line data with %d points", (int)kline_data->size());
                        
                        // 检查对象有效性
                        if (!self) {
                            ESP_LOGE(TAG, "WXT185Display instance is null");
                            delete kline_data;
                            delete kline_data_ptr;
                            return;
                        }
                        
                        try {
                            // 更新当前货币的K线数据
                            for (auto& crypto : self->crypto_data_) {
                                if (crypto.currency_id == self->current_crypto_data_.currency_id) {
                                    // 转换K线数据格式并存储
                                    crypto.kline_data_1h.clear();
                                    for (const auto& kline : *kline_data) {
                                        crypto.kline_data_1h.emplace_back(kline.open, kline.close);
                                    }
                                    break;
                                }
                            }
                            
                            // 更新图表显示
                            self->DrawKLineChart();
                        } catch (const std::exception& e) {
                            ESP_LOGE(TAG, "Exception occurred while processing K-line data: %s", e.what());
                        } catch (...) {
                            ESP_LOGE(TAG, "Unknown exception occurred while processing K-line data");
                        }
                        
                        // 清理临时分配的内存
                        delete kline_data;
                        delete kline_data_ptr;
                    }, new std::pair<WXT185Display*, std::vector<KLineData>*>(self, new std::vector<KLineData>(kline_data)));
                } catch (const std::exception& e) {
                    ESP_LOGE(TAG, "Exception occurred in K-line data callback: %s", e.what());
                } catch (...) {
                    ESP_LOGE(TAG, "Unknown exception occurred in K-line data callback");
                }
            });
            
            self->bijie_coins_connected_ = true;
        } catch (const std::exception& e) {
            ESP_LOGE(TAG, "Exception occurred while connecting to BiJie coins: %s", e.what());
        } catch (...) {
            ESP_LOGE(TAG, "Unknown exception occurred while connecting to BiJie coins");
        }
        
        // 任务完成，删除自身
        vTaskDelete(nullptr);
    }, "bijie_coins_connect", 4096, this, 5, nullptr);
}

void WXT185Display::UpdateCryptoDataFromBiJie() {
    ESP_LOGI(TAG, "Updating crypto data from BiJie service");
    if (!bijie_coins_ || !bijie_coins_connected_) {
        ESP_LOGW(TAG, "BiJie coins service not connected");
        return;
    }
    
    try {
        // 获取币界虚拟币列表
        auto coin_list = bijie_coins_->GetCoinList();
        
        // 更新我们的虚拟币数据
        for (auto& crypto : crypto_data_) {
            for (const auto& coin_info : coin_list) {
                if (crypto.symbol == coin_info.symbol) {
                    crypto.price = coin_info.price;
                    crypto.change_24h = coin_info.change_24h;
                    crypto.currency_id = coin_info.id;
                    ESP_LOGI(TAG, "Updated %s: price=%.2f, change=%.2f", 
                             crypto.symbol.c_str(), crypto.price, crypto.change_24h);
                    break;
                }
            }
        }
    } catch (const std::exception& e) {
        ESP_LOGE(TAG, "Exception occurred while updating crypto data from BiJie: %s", e.what());
    } catch (...) {
        ESP_LOGE(TAG, "Unknown exception occurred while updating crypto data from BiJie");
    }
}

void WXT185Display::SwitchCrypto(int currency_id) {
    ESP_LOGI(TAG, "Switching to crypto currency ID: %d", currency_id);
    if (!bijie_coins_) {
        ESP_LOGW(TAG, "BiJie coins service not initialized");
        return;
    }
    
    try {
        // 如果要切换到的虚拟币已经是当前虚拟币，则直接返回
        if (currency_id == current_crypto_data_.currency_id) {
            ESP_LOGI(TAG, "Already on the selected currency, no switch needed");
            return;
        }
        
        // 断开当前连接（如果当前虚拟币不是屏保虚拟币）
        if (current_crypto_data_.currency_id != screensaver_crypto_.currency_id) {
            bijie_coins_->Disconnect(current_crypto_data_.currency_id);
            ESP_LOGI(TAG, "Disconnected from previous currency ID: %d", current_crypto_data_.currency_id);
        }
        
        // 更新当前虚拟币ID
        current_crypto_data_.currency_id = currency_id;
        
        // 连接到新的虚拟币（如果新虚拟币不是屏保虚拟币）
        if (current_crypto_data_.currency_id != screensaver_crypto_.currency_id) {
            if (bijie_coins_->Connect(current_crypto_data_.currency_id)) {
                ESP_LOGI(TAG, "Switched to BiJie coins WebSocket for currency %d", current_crypto_data_.currency_id);
            } else {
                ESP_LOGE(TAG, "Failed to connect to BiJie coins WebSocket for currency %d", current_crypto_data_.currency_id);
            }
        }

        // 设置屏保虚拟币
        screensaver_crypto_ = current_crypto_data_;
        
        // 获取K线数据用于图表显示
        bijie_coins_->GetKLineData(current_crypto_data_.currency_id, 2, 30, [this](const std::vector<KLineData>& kline_data) {
            try {
                ESP_LOGI(TAG, "Received K-line data with %d points", (int)kline_data.size());
                
                // 更新当前货币的K线数据
                for (auto& crypto : crypto_data_) {
                    if (crypto.currency_id == current_crypto_data_.currency_id) {
                        // 转换K线数据格式并存储
                        crypto.kline_data_1h.clear();
                        for (const auto& kline : kline_data) {
                            crypto.kline_data_1h.emplace_back(kline.open, kline.close);
                        }
                        break;
                    }
                }
                
                // 更新图表显示
                DrawKLineChart();
            } catch (const std::exception& e) {
                ESP_LOGE(TAG, "Exception occurred while processing K-line data in SwitchCrypto: %s", e.what());
            } catch (...) {
                ESP_LOGE(TAG, "Unknown exception occurred while processing K-line data in SwitchCrypto");
            }
        });
        
        // 更新虚拟币页面内容
        UpdateCryptoData();
    } catch (const std::exception& e) {
        ESP_LOGE(TAG, "Exception occurred while switching crypto: %s", e.what());
    } catch (...) {
        ESP_LOGE(TAG, "Unknown exception occurred while switching crypto");
    }
}

void WXT185Display::SetScreensaverCrypto(int currency_id) {
    ESP_LOGI(TAG, "Setting screensaver crypto currency ID: %d", currency_id);

    if (!bijie_coins_) {
        ESP_LOGW(TAG, "BiJie coins service not initialized");
        return;
    }
    
    try {
        // 如果要设置的虚拟币已经是屏保虚拟币，则直接返回
        if (currency_id == screensaver_crypto_.currency_id) {
            ESP_LOGI(TAG, "Already set to the selected screensaver currency, no change needed");
            return;
        }
        
        // 断开之前的屏保虚拟币连接（如果之前的屏保虚拟币不是当前显示的虚拟币）
        if (screensaver_crypto_.currency_id != current_crypto_data_.currency_id) {
            bijie_coins_->Disconnect(screensaver_crypto_.currency_id);
            ESP_LOGI(TAG, "Disconnected from previous screensaver currency ID: %d", screensaver_crypto_.currency_id);
        }
        
        // 更新屏保虚拟币ID
        screensaver_crypto_.currency_id = currency_id;
        
        // 如果该虚拟币尚未连接，则连接它（如果该虚拟币不是当前显示的虚拟币）
        if (screensaver_crypto_.currency_id != current_crypto_data_.currency_id) {
            if (!bijie_coins_->IsConnected(screensaver_crypto_.currency_id)) {
                if (bijie_coins_->Connect(screensaver_crypto_.currency_id)) {
                    ESP_LOGI(TAG, "Connected to BiJie coins WebSocket for screensaver currency %d", screensaver_crypto_.currency_id);
                } else {
                    ESP_LOGE(TAG, "Failed to connect to BiJie coins WebSocket for screensaver currency %d", screensaver_crypto_.currency_id);
                }
            }
        }
        
        // 请求K线数据用于屏保显示
        bijie_coins_->GetKLineData(screensaver_crypto_.currency_id, 2, 30, [this](const std::vector<KLineData>& kline_data) {
            try {
                ESP_LOGI(TAG, "Received K-line data for screensaver with %d points", (int)kline_data.size());

                // 更新屏保关联的K线数据
                screensaver_kline_data_.clear();
                for (const auto& kline : kline_data) {
                    screensaver_kline_data_.emplace_back(kline.open, kline.close);
                }

                // 如果屏保处于激活状态，更新内容
                if (screensaver_active_) {
                    UpdateScreensaverContent();
                }
            } catch (const std::exception& e) {
                ESP_LOGE(TAG, "Exception occurred while processing K-line data in SetScreensaverCrypto: %s", e.what());
            } catch (...) {
                ESP_LOGE(TAG, "Unknown exception occurred while processing K-line data in SetScreensaverCrypto");
            }
        });

        // 如果屏保处于激活状态，更新屏保内容
        if (screensaver_active_) {
            UpdateScreensaverContent();
        }
    } catch (const std::exception& e) {
        ESP_LOGE(TAG, "Exception occurred while setting screensaver crypto: %s", e.what());
    } catch (...) {
        ESP_LOGE(TAG, "Unknown exception occurred while setting screensaver crypto");
    }
}
