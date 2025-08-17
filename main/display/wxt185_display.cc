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
#define LIGHT_CHAT_BACKGROUND_COLOR  lv_color_hex(0xF0F0F0)
#define LIGHT_USER_BUBBLE_COLOR      lv_color_hex(0x1A6C37)
#define LIGHT_ASSISTANT_BUBBLE_COLOR lv_color_hex(0xE0E0E0)
#define LIGHT_SYSTEM_BUBBLE_COLOR    lv_color_hex(0xD0D0D0)
#define LIGHT_SYSTEM_TEXT_COLOR      lv_color_hex(0x666666)
#define LIGHT_BORDER_COLOR           lv_color_hex(0xDDDDDD)
#define LIGHT_LOW_BATTERY_COLOR      lv_color_hex(0xFF0000)
#define LIGHT_HEADER_COLOR           lv_color_hex(0xEEEEEE)
#define LIGHT_SELECTOR_COLOR         lv_color_hex(0xF5F5F5)
#define LIGHT_OUTER_RING_COLOR       lv_color_hex(0xEEEEEE)  // 外圆环颜色
#define LIGHT_INNER_RING_COLOR       lv_color_hex(0xF0F0F0)  // 内圆环颜色
#define LIGHT_SCREENSAVER_SWITCH_COLOR lv_color_hex(0x1A6C37) // 屏保开关颜色

// 通用roller颜色定义 - LIGHT主题
#define LIGHT_ROLLER_BG_COLOR         lv_color_hex(0xE0F0FF)  // roller背景颜色
#define LIGHT_ROLLER_BORDER_COLOR     lv_color_hex(0x99CCFF)  // roller边框颜色

// 设置页面颜色定义 - 基于LIGHT主题
#define LIGHT_SETTINGS_LABEL_COLOR         LIGHT_TEXT_COLOR              // 设置标签颜色
#define LIGHT_SETTINGS_ROLLER_TEXT_COLOR   LIGHT_TEXT_COLOR              // 设置选择器文本颜色
#define LIGHT_SETTINGS_ROLLER_BG_COLOR     LIGHT_SELECTOR_COLOR          // 设置选择器背景颜色
#define LIGHT_SETTINGS_ROLLER_BORDER_COLOR LIGHT_BORDER_COLOR            // 设置选择器边框颜色
#define LIGHT_SETTINGS_BUTTON_TEXT_COLOR   LIGHT_TEXT_COLOR              // 设置按钮文本颜色
#define LIGHT_SETTINGS_BUTTON_BG_COLOR     LIGHT_SCREENSAVER_SWITCH_COLOR // 设置按钮背景颜色

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

// 通用roller颜色定义 - DARK主题
#define DARK_ROLLER_BG_COLOR         lv_color_hex(0x252525)  // roller背景颜色
#define DARK_ROLLER_BORDER_COLOR     lv_color_hex(0x444444)  // roller边框颜色

// 设置页面颜色定义 - 基于DARK主题
#define DARK_SETTINGS_LABEL_COLOR         DARK_TEXT_COLOR              // 设置标签颜色
#define DARK_SETTINGS_ROLLER_TEXT_COLOR   DARK_TEXT_COLOR              // 设置选择器文本颜色
#define DARK_SETTINGS_ROLLER_BG_COLOR     DARK_SELECTOR_COLOR          // 设置选择器背景颜色
#define DARK_SETTINGS_ROLLER_BORDER_COLOR DARK_BORDER_COLOR            // 设置选择器边框颜色
#define DARK_SETTINGS_BUTTON_TEXT_COLOR   DARK_TEXT_COLOR              // 设置按钮文本颜色
#define DARK_SETTINGS_BUTTON_BG_COLOR     DARK_SCREENSAVER_SWITCH_COLOR // 设置按钮背景颜色

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

// 通用roller颜色定义 - METAL主题
#define METAL_ROLLER_BG_COLOR         lv_color_hex(0xD3D3D3)  // roller背景颜色
#define METAL_ROLLER_BORDER_COLOR     lv_color_hex(0x696969)  // roller边框颜色

// 设置页面颜色定义 - 基于METAL主题
#define METAL_SETTINGS_LABEL_COLOR         METAL_TEXT_COLOR              // 设置标签颜色
#define METAL_SETTINGS_ROLLER_TEXT_COLOR   METAL_TEXT_COLOR              // 设置选择器文本颜色
#define METAL_SETTINGS_ROLLER_BG_COLOR     METAL_SELECTOR_COLOR          // 设置选择器背景颜色
#define METAL_SETTINGS_ROLLER_BORDER_COLOR METAL_BORDER_COLOR            // 设置选择器边框颜色
#define METAL_SETTINGS_BUTTON_TEXT_COLOR   METAL_TEXT_COLOR              // 设置按钮文本颜色
#define METAL_SETTINGS_BUTTON_BG_COLOR     METAL_SCREENSAVER_SWITCH_COLOR // 设置按钮背景颜色

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

// 通用roller颜色定义
#define TECHNOLOGY_ROLLER_BG_COLOR         lv_color_hex(0xE0F0FF)  // roller背景颜色
#define TECHNOLOGY_ROLLER_BORDER_COLOR     lv_color_hex(0x99CCFF)  // roller边框颜色

// 设置页面颜色定义 - 基于TECHNOLOGY主题
#define TECHNOLOGY_SETTINGS_LABEL_COLOR         TECHNOLOGY_TEXT_COLOR              // 设置标签颜色
#define TECHNOLOGY_SETTINGS_ROLLER_TEXT_COLOR   TECHNOLOGY_TEXT_COLOR              // 设置选择器文本颜色
#define TECHNOLOGY_SETTINGS_ROLLER_BG_COLOR     TECHNOLOGY_SELECTOR_COLOR          // 设置选择器背景颜色
#define TECHNOLOGY_SETTINGS_ROLLER_BORDER_COLOR TECHNOLOGY_BORDER_COLOR            // 设置选择器边框颜色
#define TECHNOLOGY_SETTINGS_BUTTON_TEXT_COLOR   TECHNOLOGY_TEXT_COLOR              // 设置按钮文本颜色
#define TECHNOLOGY_SETTINGS_BUTTON_BG_COLOR     TECHNOLOGY_SCREENSAVER_SWITCH_COLOR // 设置按钮背景颜色

// 虚拟币界面颜色定义 - 基于TECHNOLOGY主题
#define TECHNOLOGY_CRYPTO_BACKGROUND_COLOR      TECHNOLOGY_BACKGROUND_COLOR      // 背景色
#define TECHNOLOGY_CRYPTO_TEXT_COLOR            TECHNOLOGY_TEXT_COLOR            // 主文本色
#define TECHNOLOGY_CRYPTO_SUB_TEXT_COLOR        TECHNOLOGY_SYSTEM_TEXT_COLOR     // 次要文本色
#define TECHNOLOGY_CRYPTO_UP_COLOR              TECHNOLOGY_TEXT_COLOR            // 上涨颜色
#define TECHNOLOGY_CRYPTO_DOWN_COLOR            TECHNOLOGY_LOW_BATTERY_COLOR     // 下跌颜色(红色)
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

// 通用roller颜色定义
#define COSMIC_ROLLER_BG_COLOR         lv_color_hex(0x250025)  // roller背景颜色
#define COSMIC_ROLLER_BORDER_COLOR     lv_color_hex(0x800080)  // roller边框颜色

// 设置页面颜色定义 - 基于COSMIC主题
#define COSMIC_SETTINGS_LABEL_COLOR         COSMIC_TEXT_COLOR              // 设置标签颜色
#define COSMIC_SETTINGS_ROLLER_TEXT_COLOR   COSMIC_TEXT_COLOR              // 设置选择器文本颜色
#define COSMIC_SETTINGS_ROLLER_BG_COLOR     COSMIC_SELECTOR_COLOR          // 设置选择器背景颜色
#define COSMIC_SETTINGS_ROLLER_BORDER_COLOR COSMIC_BORDER_COLOR            // 设置选择器边框颜色
#define COSMIC_SETTINGS_BUTTON_TEXT_COLOR   COSMIC_TEXT_COLOR              // 设置按钮文本颜色
#define COSMIC_SETTINGS_BUTTON_BG_COLOR     COSMIC_SCREENSAVER_SWITCH_COLOR // 设置按钮背景颜色

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
    .settings_screensaver_switch = LIGHT_SCREENSAVER_SWITCH_COLOR,
    .settings_label = LIGHT_SETTINGS_LABEL_COLOR,
    .settings_button_text = LIGHT_SETTINGS_BUTTON_TEXT_COLOR,
    .settings_button_bg = LIGHT_SETTINGS_BUTTON_BG_COLOR,
    .roller_bg = LIGHT_ROLLER_BG_COLOR,
    .roller_border = LIGHT_ROLLER_BORDER_COLOR,
    .roller_text = LIGHT_TEXT_COLOR,
    .roller_indicator = LIGHT_BORDER_COLOR,
    .roller_radius = 8
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
    .settings_screensaver_switch = DARK_SCREENSAVER_SWITCH_COLOR,
    .settings_label = DARK_SETTINGS_LABEL_COLOR,
    .settings_button_text = DARK_SETTINGS_BUTTON_TEXT_COLOR,
    .settings_button_bg = DARK_SETTINGS_BUTTON_BG_COLOR,
    .roller_bg = DARK_ROLLER_BG_COLOR,
    .roller_border = DARK_ROLLER_BORDER_COLOR,
    .roller_text = DARK_TEXT_COLOR,
    .roller_indicator = DARK_BORDER_COLOR,
    .roller_radius = 8
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
    .settings_screensaver_switch = METAL_SCREENSAVER_SWITCH_COLOR,
    .settings_label = METAL_SETTINGS_LABEL_COLOR,
    .settings_button_text = METAL_SETTINGS_BUTTON_TEXT_COLOR,
    .settings_button_bg = METAL_SETTINGS_BUTTON_BG_COLOR,
    .roller_bg = METAL_ROLLER_BG_COLOR,
    .roller_border = METAL_ROLLER_BORDER_COLOR,
    .roller_text = METAL_TEXT_COLOR,
    .roller_indicator = METAL_BORDER_COLOR,
    .roller_radius = 8
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
    .settings_screensaver_switch = TECHNOLOGY_SCREENSAVER_SWITCH_COLOR,
    .settings_label = TECHNOLOGY_SETTINGS_LABEL_COLOR,
    .settings_button_text = TECHNOLOGY_SETTINGS_BUTTON_TEXT_COLOR,
    .settings_button_bg = TECHNOLOGY_SETTINGS_BUTTON_BG_COLOR,
    .roller_bg = TECHNOLOGY_ROLLER_BG_COLOR,
    .roller_border = TECHNOLOGY_ROLLER_BORDER_COLOR,
    .roller_text = TECHNOLOGY_TEXT_COLOR,
    .roller_indicator = TECHNOLOGY_BORDER_COLOR,
    .roller_radius = 8
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
    .settings_screensaver_switch = COSMIC_SCREENSAVER_SWITCH_COLOR,
    .settings_label = COSMIC_SETTINGS_LABEL_COLOR,
    .settings_button_text = COSMIC_SETTINGS_BUTTON_TEXT_COLOR,
    .settings_button_bg = COSMIC_SETTINGS_BUTTON_BG_COLOR,
    .roller_bg = COSMIC_ROLLER_BG_COLOR,
    .roller_border = COSMIC_ROLLER_BORDER_COLOR,
    .roller_text = COSMIC_TEXT_COLOR,
    .roller_indicator = COSMIC_BORDER_COLOR,
    .roller_radius = 8    
};

// 定义字体(需要idf.py menuconfig启用lvgl字体配置项)
LV_FONT_DECLARE(lv_font_montserrat_16)
LV_FONT_DECLARE(lv_font_montserrat_24)
LV_FONT_DECLARE(lv_font_montserrat_32)
LV_FONT_DECLARE(lv_font_montserrat_48)

//
LV_FONT_DECLARE(font_awesome_30_4);

// 静态成员变量定义
uint32_t WXT185Display::current_page_index_ = PAGE_CHAT;

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
    // TODO: 获取实时行情和K线行情开关状态
    
    // 保存设置到NVS
    Settings settings("display", true);
    settings.SetInt("theme_index", theme_index);
    settings.SetString("theme", ThemeString[theme_index]);
    settings.SetInt("default_crypto", crypto_index);
    settings.SetInt("kline_frequency", kline_index);
    // 修复NVS键名过长问题，将"screensaver_enabled"改为"scr_enabled"
    settings.SetInt("scr_enabled", screensaver_state ? 1 : 0);
    // TODO: 保存实时行情和K线行情开关状态
    
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
    
    // 初始化样式
    lv_style_init(&style_crypto_roller_);
    lv_style_init(&style_settings_theme_roller_);
    lv_style_init(&style_settings_default_crypto_roller_);
    lv_style_init(&style_settings_kline_time_roller_);

    // 初始化LCD屏幕
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
    // 屏幕初始化结束

        // Load theme from settings
    Settings settings("display", false);
    std::string theme_name = settings.GetString("theme", "light");
    // theme_name转为小写
    for (auto& c : theme_name) {
        c = tolower(c);
    }

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

    // 初始化默认设置
    selected_theme = settings.GetInt("theme_index", 0);       // 默认主题索引
    default_crypto = settings.GetInt("default_crypto", 0);    // 默认虚拟币索引
    kline_frequency = settings.GetInt("kline_frequency", 3);  // 默认K线频率 (3=1小时)
    screensaver_enabled = settings.GetInt("scr_enabled", 1) == 1; // 默认启用屏保
    
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

    // 创建屏保时间更新定时器
    esp_timer_create_args_t screensaver_time_timer_args = {
        .callback = ScreensaverTimeUpdateTimerCallback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "screensaver_time_update_timer",
        .skip_unhandled_events = false,
    };
    esp_timer_create(&screensaver_time_timer_args, &screensaver_time_update_timer_);
    ESP_LOGI(TAG, "Screensaver time update timer created");

    // 初始化币界虚拟币行情数据支持
    bijie_coins_connected_ = false;
    bijie_coins_ = nullptr;
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
    
    // 删除屏保时间更新定时器
    if (screensaver_time_update_timer_) {
        esp_timer_stop(screensaver_time_update_timer_);
        esp_timer_delete(screensaver_time_update_timer_);
        ESP_LOGI(TAG, "Screensaver time update timer stopped and deleted");
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

    lv_obj_set_size(main_screen_, width_ * 3, height_);  // 3倍宽度以容纳三个页面
    lv_obj_set_style_radius(main_screen_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(main_screen_, current_wxt185_theme_.background, 0);
    
    ESP_LOGI(TAG, "Created page view container");

    // 启用水平滚动
    lv_obj_set_scroll_dir(main_screen_, LV_DIR_HOR);
    lv_obj_set_scroll_snap_x(main_screen_, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scrollbar_mode(main_screen_, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(main_screen_, LV_OBJ_FLAG_SCROLLABLE);

    // 添加页面滚动回调
    lv_obj_add_event_cb(main_screen_, PageEventHandler, LV_EVENT_SCROLL_END, this);
    
#if CONFIG_ESP32_S3_TOUCH_LCD_185_WITH_TOUCH || CONFIG_ESP32_S3_TOUCH_LCD_185C_WITH_TOUCH
    ESP_LOGI(TAG, "Enabling horizontal scrolling");

    // 添加触摸事件处理（仅在有触摸屏时添加）
    lv_obj_add_event_cb(main_screen_, TouchEventHandler, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(main_screen_, TouchEventHandler, LV_EVENT_RELEASED, this);
    ESP_LOGI(TAG, "Added touch event handlers to page view");
#endif
    
    // 创建表盘圆环公共组件
    //CreateCommonComponents();
    ESP_LOGI(TAG, "Created common components");

    // 创建3个页面
    CreateChatPage();
    ESP_LOGI(TAG, "Created chat page");
    
    CreateCryptoPage();
    ESP_LOGI(TAG, "Created crypto page");
    
    CreateSettingsPage();
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
    // 设置为透明，避免挡住其他内容
    lv_obj_set_style_bg_opa(common_outer_ring_, LV_OPA_TRANSP, 0);

    /*
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
    */

}

void WXT185Display::CreateChatPage() {
    ESP_LOGI(TAG, "Creating chat page");
    
    chat_page_ = lv_obj_create(main_screen_);
    lv_obj_set_size(chat_page_, width_, height_);
    lv_obj_set_style_radius(chat_page_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(chat_page_, current_wxt185_theme_.background, 0);
    lv_obj_add_flag(chat_page_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(chat_page_, 0, 0);
    lv_obj_set_style_border_width(chat_page_, 0, 0);
    lv_obj_set_style_bg_opa(chat_page_, LV_OPA_TRANSP, 0);

    // 确保聊天页面在第一个位置
    lv_obj_set_x(chat_page_, PAGE_CHAT * width_);
    
#if CONFIG_ESP32_S3_TOUCH_LCD_185_WITH_TOUCH || CONFIG_ESP32_S3_TOUCH_LCD_185C_WITH_TOUCH
    ESP_LOGI(TAG, "Enabling horizontal scrolling to chat page begin");
    // 添加页面滚动回调
    lv_obj_add_event_cb(chat_page_, PageEventHandler, LV_EVENT_SCROLL_END, this);
    // 添加触摸事件处理（仅在有触摸屏时添加）
    lv_obj_add_event_cb(chat_page_, TouchEventHandler, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(chat_page_, TouchEventHandler, LV_EVENT_RELEASED, this);
    ESP_LOGV(TAG, "Added touch event handlers to chat page end");
#endif

    /* Status bar */
    status_bar_ = lv_obj_create(chat_page_);
    // 设置状态栏宽度为屏幕宽度的 20%
    lv_obj_set_size(status_bar_, LV_HOR_RES * 0.4f, fonts_.text_font->line_height);
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
    content_ = lv_obj_create(chat_page_);
    lv_obj_align(content_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_scrollbar_mode(content_, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(content_, 0, 0);
    lv_obj_set_width(content_, LV_HOR_RES);
    lv_obj_set_flex_grow(content_, 1);
    lv_obj_set_style_pad_all(content_, 0, 0);
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
    lv_obj_set_width(chat_message_label_, LV_HOR_RES);
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
    crypto_page_ = lv_obj_create(main_screen_);
    lv_obj_set_size(crypto_page_, width_, height_);
    lv_obj_set_style_radius(crypto_page_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(crypto_page_, current_wxt185_theme_.crypto_background, 0);
    lv_obj_add_flag(crypto_page_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(crypto_page_, 0, 0);
    lv_obj_set_style_border_width(crypto_page_, 0, 0);
    lv_obj_set_style_bg_opa(crypto_page_, LV_OPA_TRANSP, 0);

    // 水平对齐，排在第二个页面
    lv_obj_set_x(crypto_page_, PAGE_CRYPTO * width_);

#if CONFIG_ESP32_S3_TOUCH_LCD_185_WITH_TOUCH || CONFIG_ESP32_S3_TOUCH_LCD_185C_WITH_TOUCH
    ESP_LOGI(TAG, "Added touch event handlers to crypto page begin");
    // 添加页面滚动回调
    lv_obj_add_event_cb(crypto_page_, PageEventHandler, LV_EVENT_SCROLL_END, this);
    // 添加触摸事件处理（仅在有触摸屏时添加）
    lv_obj_add_event_cb(crypto_page_, TouchEventHandler, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(crypto_page_, TouchEventHandler, LV_EVENT_RELEASED, this);
    ESP_LOGV(TAG, "Added touch event handlers to crypto page end");
#endif

    // 2. 创建虚拟币roller，支持水平滚动
    crypto_roller_ = lv_roller_create(crypto_page_);
    lv_obj_set_size(crypto_roller_, 100, 100);
    // 在上面中间对齐
    lv_obj_align(crypto_roller_, LV_ALIGN_TOP_MID, 0, 10);
    
    // 使用ApplyRollerStyle方法设置样式
    ApplyRollerStyle(crypto_roller_, style_crypto_roller_);

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
        lv_roller_set_options(crypto_roller_, crypto_options, LV_ROLLER_MODE_NORMAL);
    }
    else {
        lv_roller_set_options(crypto_roller_, "", LV_ROLLER_MODE_NORMAL);
    }

    lv_roller_set_visible_row_count(crypto_roller_, 1);
    lv_obj_set_style_text_font(crypto_roller_, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(crypto_roller_, current_wxt185_theme_.text, 0);
    lv_obj_set_style_bg_color(crypto_roller_, current_wxt185_theme_.crypto_background, 0);
    lv_obj_set_style_radius(crypto_roller_, 0, 0);
    lv_obj_set_style_border_width(crypto_roller_, 0, 0);
    lv_obj_set_style_pad_all(crypto_roller_, 0, 0);
    
    // 添加事件处理函数
    lv_obj_add_event_cb(crypto_roller_, CryptoSelectorEventHandler, LV_EVENT_VALUE_CHANGED, this);

    // 创建K线频率按钮容器
    crypto_kline_btn_container_ = lv_obj_create(crypto_page_);
    lv_obj_set_size(crypto_kline_btn_container_, 210, 60); // 增加高度以容纳两行按钮
    lv_obj_align(crypto_kline_btn_container_, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_flex_flow(crypto_kline_btn_container_, LV_FLEX_FLOW_ROW_WRAP, 0); // 改为换行布局
    lv_obj_set_flex_align(crypto_kline_btn_container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START); // 调整对齐方式
    lv_obj_set_style_pad_all(crypto_kline_btn_container_, 5, 0); // 添加一些内边距
    lv_obj_set_style_border_width(crypto_kline_btn_container_, 0, 0);
    lv_obj_set_style_radius(crypto_kline_btn_container_, 0, 0);
    lv_obj_set_style_bg_opa(crypto_kline_btn_container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_flex_track_place(crypto_kline_btn_container_, LV_FLEX_ALIGN_SPACE_EVENLY, 0); // 设置子元素均匀分布

    // 添加K线频率按钮
    static const char* default_freqs[] = {"1m", "5m", "15m", "1h", "2h", "4h", "1d", "1w", "1mo", "3mo", nullptr};
    const char** klinefreq = nullptr;
    if (bijie_coins_ != nullptr) {
        klinefreq = bijie_coins_->GetKLineTimeFrequencies();
    } else {
        klinefreq = default_freqs;
    }

    // 创建10个按钮，对应10种K线频率
    for (int i = 0; i < 10; i++) {
        lv_obj_t* btn = lv_button_create(crypto_kline_btn_container_);
        kline_frequency_buttons_[i] = btn; // 保存按钮引用
        lv_obj_set_size(btn, 30, 20);
        lv_obj_set_style_radius(btn, 5, 0);
        lv_obj_set_style_bg_color(btn, current_wxt185_theme_.selector, 0);
        
        // 为选中的按钮设置不同的样式
        if (i == selected_kline_frequency_) {
            lv_obj_set_style_bg_color(btn, current_wxt185_theme_.crypto_progress_bg_color, 0); // 绿色表示选中
        }
        
        lv_obj_t* label = lv_label_create(btn);
        lv_label_set_text(label, klinefreq[i]);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(label, current_wxt185_theme_.text, 0);
        lv_obj_center(label);
        
        // 添加按钮事件处理
        lv_obj_add_event_cb(btn, KLineFrequencyButtonEventHandler, LV_EVENT_CLICKED, this);
    }

    // 显示实时行情
    crypto_content_ = lv_obj_create(crypto_page_);
    lv_obj_set_size(crypto_content_, 300, 50);
    lv_obj_set_scrollbar_mode(crypto_content_, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align_to(crypto_content_, crypto_kline_btn_container_, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_set_style_radius(crypto_content_, 10, 0);
    lv_obj_set_style_bg_color(crypto_content_, current_wxt185_theme_.crypto_background, 0);
    lv_obj_set_style_border_width(crypto_content_, 0, 0);

    crypto_price_label_ = lv_label_create(crypto_content_);
    lv_label_set_text(crypto_price_label_, "0.00");
    lv_obj_align(crypto_price_label_, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_set_style_text_font(crypto_price_label_, fonts_.text_font, 0);
    lv_obj_set_style_text_color(crypto_price_label_, current_wxt185_theme_.text, 0);

    crypto_change_label_ = lv_label_create(crypto_content_);
    lv_label_set_text(crypto_change_label_, "0.00%");
    lv_obj_align(crypto_change_label_, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_text_font(crypto_change_label_, fonts_.text_font, 0);
    lv_obj_set_style_text_color(crypto_change_label_, current_wxt185_theme_.text, 0);

    // 创建价格图表，只显示收盘价
    crypto_chart_ = lv_chart_create(crypto_page_);
    lv_obj_set_size(crypto_chart_, 300, 130);
    lv_obj_align(crypto_chart_, LV_ALIGN_BOTTOM_MID, 0, -80);
    lv_chart_set_type(crypto_chart_, LV_CHART_TYPE_LINE);

    // 不在页面创建时立即调用绘制K线图表，而是在连接到币界服务后获取数据并绘制
    // DrawKLineChart();  // 移除这行，避免在没有数据时尝试绘制
    
    // 创建更新时间标签
    lv_obj_t* update_time_label = lv_label_create(crypto_page_);
    lv_label_set_text(update_time_label, "Updated: --:--:--");
    lv_obj_set_style_text_font(update_time_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(update_time_label, current_wxt185_theme_.text, 0);
    lv_obj_align(update_time_label, LV_ALIGN_BOTTOM_MID, 0, -30);
    
    ESP_LOGI(TAG, "Crypto page creation completed");
}

void WXT185Display::CreateSettingsPage() {
    ESP_LOGI(TAG, "Creating settings page");
    
    // 1. 初始化背景
    settings_page_ = lv_obj_create(main_screen_);
    ESP_LOGI(TAG, "Settings page background created");
    lv_obj_set_size(settings_page_, width_, height_);
    lv_obj_set_style_radius(settings_page_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(settings_page_, current_wxt185_theme_.background, 0);
    lv_obj_set_style_pad_all(settings_page_, 0, 0);
    lv_obj_set_style_border_width(settings_page_, 0, 0);
    lv_obj_set_style_bg_opa(settings_page_, LV_OPA_TRANSP, 0);

    // 水平对齐，排在第三个页面
    lv_obj_set_x(settings_page_, PAGE_SETTINGS * width_);
    
#if CONFIG_ESP32_S3_TOUCH_LCD_185_WITH_TOUCH || CONFIG_ESP32_S3_TOUCH_LCD_185C_WITH_TOUCH
    ESP_LOGI(TAG, "Added touch event handlers to settings page begin");
    // 添加页面滚动回调
    lv_obj_add_event_cb(settings_page_, PageEventHandler, LV_EVENT_SCROLL_END, this);
    // 添加触摸事件处理（仅在有触摸屏时添加）
    lv_obj_add_event_cb(settings_page_, TouchEventHandler, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(settings_page_, TouchEventHandler, LV_EVENT_RELEASED, this);
    ESP_LOGV(TAG, "Added touch event handlers to settings page end");
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
    lv_obj_set_style_text_color(settings_theme_label_, current_wxt185_theme_.settings_label, 0);
    lv_obj_align(settings_theme_label_, LV_ALIGN_TOP_MID, -80, 80);

    settings_theme_roller_ = lv_roller_create(settings_page_);
    ESP_LOGI(TAG, "Theme roller created");
    lv_obj_set_style_text_font(settings_theme_roller_, &lv_font_montserrat_14, 0);
    // 使用ApplyRollerStyle方法设置样式
    ApplyRollerStyle(settings_theme_roller_, style_settings_theme_roller_);
    
    // 设置选项
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
    lv_obj_set_style_text_color(settings_default_crypto_label_, current_wxt185_theme_.settings_label, 0);
    lv_obj_align(settings_default_crypto_label_, LV_ALIGN_TOP_MID, -80, 120);

    settings_default_crypto_roller_ = lv_roller_create(settings_page_);
    ESP_LOGI(TAG, "Default crypto roller created");
    lv_obj_set_style_text_font(settings_default_crypto_roller_, &lv_font_montserrat_14, 0);
    // 使用ApplyRollerStyle方法设置样式
    ApplyRollerStyle(settings_default_crypto_roller_, style_settings_default_crypto_roller_);

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
    lv_obj_set_style_text_color(settings_kline_time_label_, current_wxt185_theme_.settings_label, 0);
    lv_obj_align(settings_kline_time_label_, LV_ALIGN_TOP_MID, -80, 160);

    settings_kline_time_roller_ = lv_roller_create(settings_page_);
    ESP_LOGI(TAG, "K-line time roller created");
    lv_obj_set_style_text_font(settings_kline_time_roller_, &lv_font_montserrat_14, 0);
    // 使用ApplyRollerStyle方法设置样式
    ApplyRollerStyle(settings_kline_time_roller_, style_settings_kline_time_roller_);
    
    
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
    lv_obj_set_style_text_color(settings_screensaver_label_, current_wxt185_theme_.settings_label, 0);
    // 将屏保开关标签放置在K线频率选择配置下方
    lv_obj_align_to(settings_screensaver_label_, settings_kline_time_label_, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);

    settings_screensaver_switch_ = lv_switch_create(settings_page_);
    ESP_LOGI(TAG, "Screensaver switch created");
    lv_obj_set_style_bg_color(settings_screensaver_switch_, current_wxt185_theme_.settings_screensaver_switch, 0);
    // 设置指示器颜色
    lv_obj_set_style_bg_color(settings_screensaver_switch_, current_wxt185_theme_.settings_screensaver_switch, LV_PART_INDICATOR | LV_STATE_CHECKED);
    if (screensaver_enabled) {
        lv_obj_add_state(settings_screensaver_switch_, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(settings_screensaver_switch_, screensaver_switch_event_handler, LV_EVENT_VALUE_CHANGED, this);
    // 将屏保开关放置在标签旁边
    lv_obj_align_to(settings_screensaver_switch_, settings_screensaver_label_, LV_ALIGN_OUT_RIGHT_MID, 30, 0);
      
    // 8. 创建保存按钮
    settings_save_button_ = lv_button_create(settings_page_);
    ESP_LOGI(TAG, "Save button created");
    // 设置按钮标签
    settings_save_label_ = lv_label_create(settings_save_button_);
    ESP_LOGI(TAG, "Save button label created");
    lv_label_set_text(settings_save_label_, "Save");
    lv_obj_center(settings_save_label_);
    
    // 设置按钮样式
    lv_obj_set_style_bg_color(settings_save_button_, current_wxt185_theme_.settings_button_bg, 0);
    lv_obj_set_style_text_color(settings_save_label_, current_wxt185_theme_.settings_button_text, 0);
    lv_obj_set_size(settings_save_button_, 100, 40);
    lv_obj_align(settings_save_button_, LV_ALIGN_BOTTOM_MID, 0, -30);

    lv_obj_add_event_cb(settings_save_button_, settings_save_button_event_handler, LV_EVENT_CLICKED, this);
    
    ESP_LOGI(TAG, "Settings page creation completed");
}

void WXT185Display::CreateScreensaverPage() {
    // 无论screensaver_enabled状态如何都创建屏保页面，只是可能不启动定时器
    ESP_LOGI(TAG, "Creating screensaver page");
    
    // 1. 创建背景
    screensaver_page_ = lv_obj_create(main_screen_);
    lv_obj_set_size(screensaver_page_, width_, height_);
    lv_obj_set_style_radius(screensaver_page_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(screensaver_page_, current_wxt185_theme_.background, 0);
    // 设置可滚动（用于退出屏保）
    lv_obj_add_flag(screensaver_page_, LV_OBJ_FLAG_SCROLLABLE);
    // 隐藏滚动条
    lv_obj_set_scrollbar_mode(screensaver_page_, LV_SCROLLBAR_MODE_OFF);
    
#if CONFIG_ESP32_S3_TOUCH_LCD_185_WITH_TOUCH || CONFIG_ESP32_S3_TOUCH_LCD_185C_WITH_TOUCH
    ESP_LOGI(TAG, "Added touch event handlers to screensaver page start");
    // 为屏保页面添加触摸事件处理程序
    lv_obj_add_event_cb(screensaver_page_, TouchEventHandler, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(screensaver_page_, TouchEventHandler, LV_EVENT_RELEASED, this);
    ESP_LOGI(TAG, "Added touch event handlers to screensaver page end");
#endif
    
    // 2. 创建外圆环（刻度环）
    screensaver_outer_ring_ = lv_obj_create(screensaver_page_);
    lv_obj_set_size(screensaver_outer_ring_, width_ - 20 , width_ - 20);
    lv_obj_center(screensaver_outer_ring_);
    lv_obj_set_style_radius(screensaver_outer_ring_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(screensaver_outer_ring_, current_wxt185_theme_.outer_ring_background, 0);
    lv_obj_set_style_border_width(screensaver_outer_ring_, 2, 0);
    lv_obj_set_style_border_color(screensaver_outer_ring_, current_wxt185_theme_.border, 0);

    // 5. 创建币名显示
    screensaver_crypto_name_ = lv_label_create(screensaver_page_);
    lv_label_set_text_fmt(screensaver_crypto_name_, "%s", screensaver_crypto_.symbol.c_str());
    lv_obj_set_style_text_font(screensaver_crypto_name_, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(screensaver_crypto_name_, current_wxt185_theme_.text, 0);
    lv_obj_align(screensaver_crypto_name_, LV_ALIGN_CENTER, 0, -60);

    // 6. 创建全称显示
    screensaver_crypto_fullname_ = lv_label_create(screensaver_page_);
    lv_label_set_text_fmt(screensaver_crypto_fullname_, "%s", screensaver_crypto_.name.c_str());
    lv_obj_set_style_text_font(screensaver_crypto_fullname_, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(screensaver_crypto_fullname_, current_wxt185_theme_.crypto_sub_text, 0);
    lv_obj_align_to(screensaver_crypto_fullname_, screensaver_crypto_name_, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

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
    lv_obj_set_style_text_font(screensaver_time_, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(screensaver_time_, current_wxt185_theme_.crypto_sub_text, 0);
    lv_obj_align(screensaver_time_, LV_ALIGN_BOTTOM_MID, 0, -30);

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
    if (!crypto_page_ || !crypto_header_ || !crypto_roller_ || !crypto_content_ || 
        !crypto_price_label_ || !crypto_change_label_ || !crypto_chart_) return;
    ESP_LOGI(TAG, "Applying crypto page theme");
    
    // 应用页面背景
    lv_obj_set_style_bg_color(crypto_page_, current_wxt185_theme_.background, 0);
    
    // 应用头部区域主题
    lv_obj_set_style_text_color(crypto_header_, current_wxt185_theme_.text, 0);
    lv_obj_set_style_bg_color(crypto_header_, current_wxt185_theme_.header, 0);
        
    // 应用价格和变化率标签主题
    lv_obj_set_style_text_color(crypto_price_label_, current_wxt185_theme_.crypto_text, 0);
    lv_obj_set_style_text_color(crypto_change_label_, current_wxt185_theme_.crypto_text, 0);
    
    // 应用图表区域主题
    lv_obj_set_style_bg_color(crypto_chart_, current_wxt185_theme_.selector, 0);
    lv_obj_set_style_border_color(crypto_chart_, current_wxt185_theme_.border, 0);
    
    // 更新K线频率按钮的主题
    for (int i = 0; i < 10; i++) {
        if (kline_frequency_buttons_[i]) {
            if (i == selected_kline_frequency_) {
                // 选中的按钮使用特殊颜色
                lv_obj_set_style_bg_color(kline_frequency_buttons_[i], lv_color_hex(0x1a6c37), 0);
            } else {
                // 未选中的按钮使用默认选择器颜色
                lv_obj_set_style_bg_color(kline_frequency_buttons_[i], current_wxt185_theme_.selector, 0);
            }
        }
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
    
    // 应用标签文本颜色
    if (settings_theme_label_) {
        lv_obj_set_style_text_color(settings_theme_label_, current_wxt185_theme_.settings_label, 0);
    }
    
    if (settings_default_crypto_label_) {
        lv_obj_set_style_text_color(settings_default_crypto_label_, current_wxt185_theme_.settings_label, 0);
    }
    
    if (settings_kline_time_label_) {
        lv_obj_set_style_text_color(settings_kline_time_label_, current_wxt185_theme_.settings_label, 0);
    }
    
    if (settings_screensaver_label_) {
        lv_obj_set_style_text_color(settings_screensaver_label_, current_wxt185_theme_.settings_label, 0);
    }
    
    // 应用屏保开关主题
    if (settings_screensaver_switch_) {
        lv_obj_set_style_bg_color(settings_screensaver_switch_, current_wxt185_theme_.settings_screensaver_switch, 0);
        // 设置指示器颜色
        lv_obj_set_style_bg_color(settings_screensaver_switch_, current_wxt185_theme_.settings_screensaver_switch, LV_PART_INDICATOR | LV_STATE_CHECKED);
    }
    
    // 应用保存按钮主题
    if (settings_save_button_) {
        lv_obj_set_style_bg_color(settings_save_button_, current_wxt185_theme_.settings_button_bg, 0);
    }
    
    // 应用保存按钮标签主题
    if (settings_save_label_) {
        lv_obj_set_style_text_color(settings_save_label_, current_wxt185_theme_.settings_button_text, 0);
    }
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
        // 默认文本颜色
        lv_obj_set_style_text_color(chat_message_label_, current_wxt185_theme_.text, 0);
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
    
    // 更新当前选中的虚拟币信息（页面顶部的详细信息）
    if (crypto_price_label_ && crypto_change_label_ && !crypto_data_.empty()) {
        // 获取当前选中的虚拟币（根据roller的选项）
        int selected_index = lv_roller_get_selected(crypto_roller_);
        if (selected_index >= 0 && selected_index < (int)crypto_data_.size()) {
            const CryptocurrencyData& selected_crypto = crypto_data_[selected_index];
            
            // 更新价格标签
            char price_text[32];
            snprintf(price_text, sizeof(price_text), "$%.2f", selected_crypto.price);
            lv_label_set_text(crypto_price_label_, price_text);
            
            // 更新变化率标签
            char change_text[32];
            snprintf(change_text, sizeof(change_text), "%.2f%%", selected_crypto.change_24h);
            lv_label_set_text(crypto_change_label_, change_text);
            
            // 根据变化率设置颜色
            if (selected_crypto.change_24h >= 0) {
                lv_obj_set_style_text_color(crypto_change_label_, current_wxt185_theme_.crypto_up_color, 0);
            } else {
                lv_obj_set_style_text_color(crypto_change_label_, current_wxt185_theme_.crypto_down_color, 0);
            }
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
            // 显示提示信息
            lv_obj_t* no_data_label = lv_label_create(crypto_chart_);
            lv_label_set_text(no_data_label, "Service not available");
            lv_obj_center(no_data_label);
            return;
        }
        
        // 获取当前货币的K线数据
        auto market_data = bijie_coins_->GetMarketData(current_crypto_data_.currency_id);
        if (!market_data) {
            ESP_LOGW(TAG, "No market data available for currency ID: %d", current_crypto_data_.currency_id);
            // 显示提示信息
            lv_obj_t* no_data_label = lv_label_create(crypto_chart_);
            lv_label_set_text(no_data_label, "No data available");
            lv_obj_center(no_data_label);
            return;
        }

        // 输出当前最新的数据
        ESP_LOGI(TAG, "Latest market data: %s", market_data->toString().c_str());
        
        // 根据选择的K线频率获取相应的数据
        const std::vector<KLineData>* kline_data = nullptr;
        const char* frequency_names[] = {"1m", "5m", "15m", "1h", "2h", "4h", "1d", "1w", "1mo", "3mo"};
        
        switch (selected_kline_frequency_) {
            case 0: // 1分钟
                kline_data = &market_data->kline_data_1m;
                break;
            case 1: // 5分钟
                kline_data = &market_data->kline_data_5m;
                break;
            case 2: // 15分钟
                kline_data = &market_data->kline_data_15m;
                break;
            case 3: // 1小时 (默认)
                kline_data = &market_data->kline_data_1h;
                break;
            case 4: // 2小时
                kline_data = &market_data->kline_data_2h;
                break;
            case 5: // 4小时
                kline_data = &market_data->kline_data_4h;
                break;
            case 6: // 1天
                kline_data = &market_data->kline_data_1d;
                break;
            case 7: // 1周
                kline_data = &market_data->kline_data_1w;
                break;
            case 8: // 1月
                kline_data = &market_data->kline_data_1mo;
                break;
            case 9: // 3月
                kline_data = &market_data->kline_data_3mo;
                break;
            default:
                kline_data = &market_data->kline_data_1h; // 默认使用1小时
                break;
        }
        
        // 检查是否有K线数据
        if (kline_data->empty()) {
            ESP_LOGW(TAG, "No K-line data available for currency ID: %d, frequency: %s", 
                     current_crypto_data_.currency_id, frequency_names[selected_kline_frequency_]);
            // 显示提示信息
            lv_obj_t* no_data_label = lv_label_create(crypto_chart_);
            lv_label_set_text(no_data_label, "Loading data...");
            lv_obj_center(no_data_label);
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
        
        // 设置图表点数为实际数据点数量，但不超过最大显示数量30
        uint16_t chart_point_count = kline_data->size() > 30 ? 30 : kline_data->size();
        lv_chart_set_point_count(chart, chart_point_count);
        
        // 设置图表样式
        lv_chart_set_type(chart, LV_CHART_TYPE_LINE); // 使用线图替代K线图，因为LVGL不直接支持K线图
        lv_chart_set_div_line_count(chart, 5, 5);
        
        // 设置图表样式
        lv_obj_set_style_bg_color(chart, current_wxt185_theme_.background, 0);
        lv_obj_set_style_border_color(chart, current_wxt185_theme_.border, 0);
        lv_obj_set_style_text_color(chart, current_wxt185_theme_.text, 0);
        
        // 计算Y轴范围以适配数据
        float min_price = std::numeric_limits<float>::max();
        float max_price = std::numeric_limits<float>::lowest();
        
        // 遍历所有数据点找到最大值和最小值
        for (const auto& kline_point : *kline_data) {
            // 检查high, low, open, close中的最大值和最小值
            float high = kline_point.high;
            float low = kline_point.low;
            
            if (high > max_price) max_price = high;
            if (low < min_price) min_price = low;
        }
        
        // 添加一些边距，使图表不紧贴边缘
        float margin = (max_price - min_price) * 0.1f; // 10%的边距
        if (margin == 0) margin = 1.0f; // 防止价格完全相同时边距为0
        
        min_price -= margin;
        max_price += margin;
        
        // 设置Y轴范围
        lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 
                          static_cast<int32_t>(min_price), 
                          static_cast<int32_t>(max_price));
        
        ESP_LOGI(TAG, "K-line chart Y-axis range: min=%.2f, max=%.2f", min_price, max_price);
        
        // 添加数据系列 - 显示收盘价
        lv_chart_series_t* close_ser = lv_chart_add_series(chart, lv_color_hex(0x00FF00), LV_CHART_AXIS_PRIMARY_Y);
        if (!close_ser) {
            ESP_LOGE(TAG, "Failed to add chart series");
            return;
        }
        
        // 添加点到图表
        int point_count = 0;
        // 为了更好地显示图表，我们按时间顺序添加数据（从 oldest 到 newest）
        for (int i = kline_data->size() - 1; i >= 0 && point_count < 30; i--) { // 限制显示30个点
            // 只添加收盘价到图表
            lv_chart_set_next_value(chart, close_ser, static_cast<int32_t>(kline_data->at(i).close)); 
            point_count++;
        }
        
        ESP_LOGI(TAG, "Added %d points to K-line chart for frequency %s", 
                 point_count, frequency_names[selected_kline_frequency_]);
    
        
        // 更新时间显示
        lv_obj_t* update_time_label = lv_obj_get_child(crypto_page_, -1); // 获取最后一个子对象，即更新时间标签
        if (update_time_label) {
            // 获取当前时间
            time_t now;
            time(&now);
            struct tm timeinfo;
            localtime_r(&now, &timeinfo);
            
            char time_str[64];
            strftime(time_str, sizeof(time_str), "Updated: %H:%M:%S", &timeinfo);
            lv_label_set_text(update_time_label, time_str);
        }
    } catch (const std::exception& e) {
        ESP_LOGE(TAG, "Exception occurred while drawing K-line chart: %s", e.what());
        // 显示错误信息
        lv_obj_t* error_label = lv_label_create(crypto_chart_);
        lv_label_set_text(error_label, "Error loading chart");
        lv_obj_center(error_label);
    } catch (...) {
        ESP_LOGE(TAG, "Unknown exception occurred while drawing K-line chart");
        // 显示错误信息
        lv_obj_t* error_label = lv_label_create(crypto_chart_);
        lv_label_set_text(error_label, "Unknown error");
        lv_obj_center(error_label);
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
    
    // 如果当前处于屏保状态，任何触摸都应关闭屏保
    if (screensaver_active_) {
        ESP_LOGI(TAG, "Touch detected in screensaver mode, exiting screensaver");
        OnActivity();
        return;
    }
    
    // 计算滑动距离
    int16_t diff_x = point.x - touch_start_point_.x;
    int16_t diff_y = point.y - touch_start_point_.y;
    
    // 判断是否为水平滑动且距离足够
    if (abs(diff_x) > abs(diff_y) && abs(diff_x) > 30) {
        if (diff_x > 0) {
            // 向右滑动，切换到上一个页面
            ESP_LOGI(TAG, "Swipe right detected, switching to previous page");
            if (current_page_index_ > PAGE_CHAT) {
                SwitchToPage(current_page_index_ - 1);
            }
        } else {
            // 向左滑动，切换到下一个页面
            ESP_LOGI(TAG, "Swipe left detected, switching to next page");
            if (current_page_index_ < PAGE_SETTINGS) {
                SwitchToPage(current_page_index_ + 1);
            }
        }
    } else if (abs(diff_x) <= 10 && abs(diff_y) <= 10) {
        // 点击而非滑动，将事件视为用户活动
        ESP_LOGI(TAG, "Touch tap detected, updating activity");
        OnActivity();
    }
}

void WXT185Display::SwitchToPage(int page_index) {
    ESP_LOGI(TAG, "Switching to page %d", page_index);
    DisplayLockGuard lock(this);
    if (main_screen_ == nullptr || page_index < 0 || page_index > 2) return;
    
    // 如果当前在屏保状态，不执行页面切换
    if (screensaver_active_) {
        ESP_LOGI(TAG, "Currently in screensaver mode, ignoring page switch");
        return;
    }
    
    // 如果目标页面和当前页面相同，直接返回
    if (current_page_index_ == page_index) {
        return;
    }
    
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
    
    // 如果切换到虚拟币页面，更新显示
    if (page_index == PAGE_CRYPTO) {
        UpdateCryptoData();
    }
}

void WXT185Display::PageEventHandler(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    WXT185Display* self = static_cast<WXT185Display*>(lv_event_get_user_data(e));
     
    if (code == LV_EVENT_SCROLL_END) {
        // 如果当前在屏保状态，不处理页面滚动事件
        if (self->screensaver_active_) {
            ESP_LOGI(TAG, "Currently in screensaver mode, exiting screensaver due to scroll");
            self->OnActivity();
            return;
        }
        
        lv_point_t scroll_end;
        lv_obj_get_scroll_end(self->main_screen_, &scroll_end);
        int16_t scroll_end_x = scroll_end.x;

        // 根据滚动结束位置确定当前页面
        int new_page_index = (scroll_end_x + self->width_ / 2) / self->width_;
        if (new_page_index < 0) new_page_index = 0;
        if (new_page_index > 2) new_page_index = 2;

        self->current_page_index_ = new_page_index;
        
        // 注意：不再需要滚动到指定页面，因为用户已经完成了滚动操作
        // 只需要更新当前页面索引并执行相应操作
        
        // 如果切换到虚拟币页面，更新显示
        if (new_page_index == PAGE_CRYPTO) {
            self->UpdateCryptoData();
        }
        
        ESP_LOGI(TAG, "Page switched to index: %d", new_page_index);
    }
}

void WXT185Display::KLineFrequencyButtonEventHandler(lv_event_t* e) {
    ESP_LOGI(TAG, "KLine frequency button event handler called");
    lv_event_code_t code = lv_event_get_code(e);
    WXT185Display* self = static_cast<WXT185Display*>(lv_event_get_user_data(e));
    
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t* btn = (lv_obj_t* )lv_event_get_target(e);
        
        // 查找被点击的按钮是哪一个 (修复循环次数，确保所有10个按钮都能被处理)
        for (int i = 0; i < 10; i++) {
            if (self->kline_frequency_buttons_[i] == btn) {
                ESP_LOGI(TAG, "KLine frequency button %d clicked", i);
                
                // 更新选中状态
                self->selected_kline_frequency_ = i;
       
                // 更新所有按钮的样式
                for (int j = 0; j < 10; j++) {
                    if (j == i) {
                        // 选中的按钮设置为绿色
                        lv_obj_set_style_bg_color(self->kline_frequency_buttons_[j], lv_color_hex(0x1a6c37), 0);
                    } else {
                        // 未选中的按钮设置为默认颜色
                        lv_obj_set_style_bg_color(self->kline_frequency_buttons_[j], self->current_wxt185_theme_.selector, 0);
                    }
                }
                
                // 请求新的K线数据
                if (self->bijie_coins_ && self->bijie_coins_connected_) {
                    uint32_t kline_type = self->GetKLineTypeByIndex(i);
                    ESP_LOGI(TAG, "Requesting K-line data for currency %d with type %d", self->current_crypto_data_.currency_id, kline_type);
                    self->bijie_coins_->GetKLineData(self->current_crypto_data_.currency_id, kline_type, 30, [self](const std::vector<KLineData>& kline_data) {
                        ESP_LOGI(TAG, "Received K-line data with %d points", kline_data.size());
                        // 数据获取完成后重新绘制图表
                        self->DrawKLineChart();
                    });
                } else {
                    // 如果没有连接到币界服务，直接重新绘制图表（使用已有数据）
                    self->DrawKLineChart();
                }
                break;
            }
        }
    }
}

void WXT185Display::CryptoSelectorEventHandler(lv_event_t* e) {
    ESP_LOGI(TAG, "Crypto selector event handler called");
    
    lv_event_code_t code = lv_event_get_code(e);
    WXT185Display* self = static_cast<WXT185Display*>(lv_event_get_user_data(e));

    if (code == LV_EVENT_VALUE_CHANGED) {
        // 虚拟币选择事件处理
        if (self && self->crypto_roller_) {
            int selected_index = lv_roller_get_selected(self->crypto_roller_);
            ESP_LOGI(TAG, "Crypto roller value changed to index: %d", selected_index);
            
            if (selected_index >= 0 && selected_index < (int)self->crypto_data_.size()) {
                // 更新当前选中的虚拟币数据
                self->current_crypto_data_ = self->crypto_data_[selected_index];
                ESP_LOGI(TAG, "Updated current crypto data to: %s (ID: %d)", 
                         self->current_crypto_data_.symbol.c_str(), 
                         self->current_crypto_data_.currency_id);
                
                // 更新UI显示
                self->UpdateCryptoData();
                
                // 如果已连接到币界服务，获取新的K线数据
                if (self->bijie_coins_ && self->bijie_coins_connected_) {
                    uint32_t kline_type = self->GetKLineTypeByIndex(self->selected_kline_frequency_);
                    ESP_LOGI(TAG, "Requesting K-line data for new crypto (ID: %d) with type %d", 
                             self->current_crypto_data_.currency_id, kline_type);
                    self->bijie_coins_->GetKLineData(self->current_crypto_data_.currency_id, kline_type, 30, 
                                                     [self](const std::vector<KLineData>& kline_data) {
                        ESP_LOGI(TAG, "Received K-line data with %d points for new crypto", kline_data.size());
                        self->DrawKLineChart();
                    });
                }
            }
        }
    }
}

void WXT185Display::ThemeSelectorEventHandler(lv_event_t* e) {
    ESP_LOGI(TAG, "Theme selector event handler called");
    
    lv_event_code_t code = lv_event_get_code(e);
    WXT185Display* self = static_cast<WXT185Display*>(lv_event_get_user_data(e));
    
    if (code == LV_EVENT_VALUE_CHANGED) {
        if (self && self->settings_theme_roller_) {
            int selected_theme = lv_roller_get_selected(self->settings_theme_roller_);
            ESP_LOGI(TAG, "Theme roller value changed to index: %d", selected_theme);
            
            // 根据选择的主题索引设置当前主题
            switch (selected_theme) {
                case 0: // Light theme
                    self->current_wxt185_theme_ = LIGHT_THEME_WXT185;
                    break;
                case 1: // Dark theme
                    self->current_wxt185_theme_ = DARK_THEME_WXT185;
                    break;
                case 2: // Metal theme
                    self->current_wxt185_theme_ = METAL_THEME_WXT185;
                    break;
                case 3: // Technology theme
                    self->current_wxt185_theme_ = TECHNOLOGY_THEME_WXT185;
                    break;
                case 4: // Cosmic theme
                    self->current_wxt185_theme_ = COSMIC_THEME_WXT185;
                    break;
                default:
                    self->current_wxt185_theme_ = LIGHT_THEME_WXT185;
                    break;
            }
            
            // 应用新主题
            self->ApplyTheme();
        }
    }
}

void WXT185Display::TimeframeSelectorEventHandler(lv_event_t* e) {
    ESP_LOGI(TAG, "Timeframe selector event handler called");
    
    lv_event_code_t code = lv_event_get_code(e);
    WXT185Display* self = static_cast<WXT185Display*>(lv_event_get_user_data(e));
    
    if (code == LV_EVENT_VALUE_CHANGED) {
        if (self) {
            // 获取触发事件的对象（时间框架选择器）
            lv_obj_t* roller = (lv_obj_t*)lv_event_get_target(e);
            if (roller) {
                // 获取选中的时间框架索引
                int selected_timeframe = lv_roller_get_selected(roller);
                ESP_LOGI(TAG, "Timeframe roller value changed to index: %d", selected_timeframe);
                
                // 更新当前选中的时间框架
                self->selected_kline_frequency_ = selected_timeframe;
                
                // 更新所有K线频率按钮的样式
                for (int i = 0; i < 10; i++) {
                    if (self->kline_frequency_buttons_[i]) {
                        if (i == selected_timeframe) {
                            // 选中的按钮使用特殊颜色
                            lv_obj_set_style_bg_color(self->kline_frequency_buttons_[i], lv_color_hex(0x1a6c37), 0);
                        } else {
                            // 未选中的按钮使用默认选择器颜色
                            lv_obj_set_style_bg_color(self->kline_frequency_buttons_[i], self->current_wxt185_theme_.selector, 0);
                        }
                    }
                }
                
                // 如果已连接到币界服务，获取新的K线数据
                if (self->bijie_coins_ && self->bijie_coins_connected_) {
                    uint32_t kline_type = self->GetKLineTypeByIndex(selected_timeframe);
                    ESP_LOGI(TAG, "Requesting K-line data for currency %d with type %d", 
                             self->current_crypto_data_.currency_id, kline_type);
                    
                    self->bijie_coins_->GetKLineData(self->current_crypto_data_.currency_id, kline_type, 30, 
                                                     [self](const std::vector<KLineData>& kline_data) {
                        ESP_LOGI(TAG, "Received K-line data with %d points", (int)kline_data.size());
                        // 数据获取完成后重新绘制图表
                        self->DrawKLineChart();
                    });
                } else {
                    // 如果没有连接到币界服务，直接重新绘制图表（使用已有数据）
                    self->DrawKLineChart();
                }
            }
        }
    }
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

// 添加友元函数用于LVGL异步调用
static void update_crypto_data_async(void* user_data) {
    // 检查参数有效性
    if (!user_data) {
        ESP_LOGE(TAG, "update_crypto_data_async: user_data is null");
        return;
    }
    
    try {
        WXT185Display* self = static_cast<WXT185Display*>(user_data);
        
        // 再次检查对象有效性
        if (!self) {
            ESP_LOGE(TAG, "update_crypto_data_async: self is null after cast");
            return;
        }

        // 检查网络是否就绪，如果未就绪则直接返回，不发出HTTP请求
        if (!self->WaitForNetworkReady(0)) {  // 0表示立即返回，不等待
            ESP_LOGW(TAG, "Network not ready, skipping crypto data update");
            return;
        }

        // 根据控制变量决定是否获取实时行情
        if (self->enable_realtime_crypto_data_) {
            // 触发Connect
            if (!self->bijie_coins_connected_) {
                self->ConnectToBiJieCoins();
            }
            
            // 更新虚拟币数据
            self->UpdateCryptoDataFromBiJie();
        }
        
        // 如果屏保处于激活状态，更新屏保内容
        if (self->screensaver_active_) {
            self->UpdateScreensaverContent();
        }
        
        // 如果当前在虚拟币页面，更新虚拟币页面内容
        if (self->current_page_index_ == PAGE_CRYPTO) {  // 1是虚拟币页面索引
            self->UpdateCryptoData();
            
            // 根据控制变量决定是否获取K线数据
            if (self->enable_kline_crypto_data_) {
                // 更新K线图表
                self->DrawKLineChart();
            }
        }
    } catch (const std::exception& e) {
        ESP_LOGE(TAG, "Exception occurred in update_crypto_data_async: %s", e.what());
    } catch (...) {
        ESP_LOGE(TAG, "Unknown exception occurred in update_crypto_data_async");
    }
}

void WXT185Display::CryptoUpdateTimerCallback(void* arg) {
    // 检查参数有效性
    if (!arg) {
        ESP_LOGE(TAG, "CryptoUpdateTimerCallback: arg is null");
        return;
    }
    
    WXT185Display* self = static_cast<WXT185Display*>(arg);
    
    try {
        // 检查是否启用了币界虚拟币行情数据或屏保功能
        if ((self->bijie_coins_ && self->bijie_coins_connected_) || screensaver_enabled) {
            ESP_LOGI(TAG, "Crypto update timer triggered");
            
            // 使用LVGL异步调用来更新UI，确保在LVGL线程中执行
            lv_async_call(update_crypto_data_async, self);
        }
    } catch (const std::exception& e) {
        ESP_LOGE(TAG, "Exception occurred in CryptoUpdateTimerCallback: %s", e.what());
    } catch (...) {
        ESP_LOGE(TAG, "Unknown exception occurred in CryptoUpdateTimerCallback");
    }
    
    // 重新启动定时器，每5秒更新一次行情
    if (self->crypto_update_timer_) {
        esp_timer_start_once(self->crypto_update_timer_, 5 * 1000 * 1000); // 5秒
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

// 屏保时间更新定时器回调函数
void WXT185Display::ScreensaverTimeUpdateTimerCallback(void* arg) {
    WXT185Display* self = static_cast<WXT185Display*>(arg);
    
    // 使用LVGL异步调用来更新UI，确保在LVGL线程中执行
    lv_async_call([](void* user_data) {
        WXT185Display* self = static_cast<WXT185Display*>(user_data);
        if (self && self->screensaver_active_) {
            // 只更新时间显示
            self->UpdateScreensaverTime();
        }
    }, self);
}

void WXT185Display::StartScreensaverTimeUpdateTimer() {
    if (screensaver_time_update_timer_) {
        esp_timer_stop(screensaver_time_update_timer_);
        // 每秒更新一次时间
        esp_timer_start_periodic(screensaver_time_update_timer_, 1000000); // 1秒
        ESP_LOGI(TAG, "Screensaver time update timer started");
    }
}

void WXT185Display::StopScreensaverTimeUpdateTimer() {
    if (screensaver_time_update_timer_) {
        esp_timer_stop(screensaver_time_update_timer_);
        ESP_LOGI(TAG, "Screensaver time update timer stopped");
    }
}

// 更新屏保时间显示
void WXT185Display::UpdateScreensaverTime() {
    DisplayLockGuard lock(this);
    
    if (!screensaver_active_) return;

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
        
        // 启动屏保时间更新定时器
        StartScreensaverTimeUpdateTimer();
        
        // 隐藏当前页面
        if (chat_page_) lv_obj_add_flag(chat_page_, LV_OBJ_FLAG_HIDDEN);
        if (crypto_page_) lv_obj_add_flag(crypto_page_, LV_OBJ_FLAG_HIDDEN);
        if (settings_page_) lv_obj_add_flag(settings_page_, LV_OBJ_FLAG_HIDDEN);
        
        // 显示屏保页面
        if (screensaver_page_) {
            lv_obj_clear_flag(screensaver_page_, LV_OBJ_FLAG_HIDDEN);
            // 确保屏保页面在最前
            lv_obj_move_foreground(screensaver_page_);
            // 强制刷新屏保页面内容
            UpdateScreensaverContent();
            UpdateScreensaverTime();
        }
        
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
        
        // 停止屏保时间更新定时器
        StopScreensaverTimeUpdateTimer();
        
        // 隐藏屏保页面
        if (screensaver_page_) lv_obj_add_flag(screensaver_page_, LV_OBJ_FLAG_HIDDEN);
        
        // 显示主屏幕并确保启用滚动功能
        if (main_screen_) {
            lv_obj_clear_flag(main_screen_, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(main_screen_);
            
            // 重新启用主屏幕的滚动功能
            lv_obj_set_scroll_dir(main_screen_, LV_DIR_HOR);
            lv_obj_set_scroll_snap_x(main_screen_, LV_SCROLL_SNAP_CENTER);
            lv_obj_set_scrollbar_mode(main_screen_, LV_SCROLLBAR_MODE_OFF);
            lv_obj_add_flag(main_screen_, LV_OBJ_FLAG_SCROLLABLE);
            
            // 确保主屏幕有正确的事件处理回调
#if CONFIG_ESP32_S3_TOUCH_LCD_185_WITH_TOUCH || CONFIG_ESP32_S3_TOUCH_LCD_185C_WITH_TOUCH
            // 先移除可能存在的旧事件回调以避免重复
            lv_obj_remove_event_cb(main_screen_, PageEventHandler);
            // 添加页面滚动回调
            lv_obj_add_event_cb(main_screen_, PageEventHandler, LV_EVENT_SCROLL_END, this);
            
            // 重新添加触摸事件处理回调
            lv_obj_remove_event_cb(main_screen_, TouchEventHandler);
            lv_obj_add_event_cb(main_screen_, TouchEventHandler, LV_EVENT_PRESSED, this);
            lv_obj_add_event_cb(main_screen_, TouchEventHandler, LV_EVENT_RELEASED, this);
            
            // 移除各页面的滚动事件回调以避免冲突
            if (chat_page_) {
                lv_obj_remove_event_cb(chat_page_, PageEventHandler);
            }
            if (crypto_page_) {
                lv_obj_remove_event_cb(crypto_page_, PageEventHandler);
            }
            if (settings_page_) {
                lv_obj_remove_event_cb(settings_page_, PageEventHandler);
            }
            
            // 重新添加各页面的事件处理回调
            if (chat_page_) {
                // 重新添加页面滚动回调
                lv_obj_add_event_cb(chat_page_, PageEventHandler, LV_EVENT_SCROLL_END, this);
                
                // 重新添加触摸事件处理回调
                lv_obj_remove_event_cb(chat_page_, TouchEventHandler);
                lv_obj_add_event_cb(chat_page_, TouchEventHandler, LV_EVENT_PRESSED, this);
                lv_obj_add_event_cb(chat_page_, TouchEventHandler, LV_EVENT_RELEASED, this);
            }
            if (crypto_page_) {
                // 重新添加页面滚动回调
                lv_obj_add_event_cb(crypto_page_, PageEventHandler, LV_EVENT_SCROLL_END, this);
                
                // 重新添加触摸事件处理回调
                lv_obj_remove_event_cb(crypto_page_, TouchEventHandler);
                lv_obj_add_event_cb(crypto_page_, TouchEventHandler, LV_EVENT_PRESSED, this);
                lv_obj_add_event_cb(crypto_page_, TouchEventHandler, LV_EVENT_RELEASED, this);
            }
            if (settings_page_) {
                // 重新添加页面滚动回调
                lv_obj_add_event_cb(settings_page_, PageEventHandler, LV_EVENT_SCROLL_END, this);
                
                // 重新添加触摸事件处理回调
                lv_obj_remove_event_cb(settings_page_, TouchEventHandler);
                lv_obj_add_event_cb(settings_page_, TouchEventHandler, LV_EVENT_PRESSED, this);
                lv_obj_add_event_cb(settings_page_, TouchEventHandler, LV_EVENT_RELEASED, this);
            }
#endif
            
            // 确保当前页面正确显示
            switch (current_page_index_) {
                case PAGE_CHAT:
                    if (chat_page_) {
                        lv_obj_clear_flag(chat_page_, LV_OBJ_FLAG_HIDDEN);
                        lv_obj_scroll_to_view_recursive(chat_page_, LV_ANIM_OFF);
                    }
                    break;
                case PAGE_CRYPTO:
                    if (crypto_page_) {
                        lv_obj_clear_flag(crypto_page_, LV_OBJ_FLAG_HIDDEN);
                        lv_obj_scroll_to_view_recursive(crypto_page_, LV_ANIM_OFF);
                    }
                    break;
                case PAGE_SETTINGS:
                    if (settings_page_) {
                        lv_obj_clear_flag(settings_page_, LV_OBJ_FLAG_HIDDEN);
                        lv_obj_scroll_to_view_recursive(settings_page_, LV_ANIM_OFF);
                    }
                    break;
            }
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

    // 注意：这里不再更新时间，因为时间更新现在由独立的定时器处理
    // 更新虚拟币简称
    if (screensaver_crypto_name_) {
        lv_label_set_text(screensaver_crypto_name_, screensaver_crypto_.symbol.c_str());
    }

    // 更新虚拟币全称
    if (screensaver_crypto_fullname_) {
        lv_label_set_text(screensaver_crypto_fullname_, screensaver_crypto_.name.c_str());
    }
    
    // 检查币界服务是否已初始化
    if (bijie_coins_) {
        try {
            // 从币界获取屏保虚拟币数据
            auto market_data = bijie_coins_->GetMarketData(screensaver_crypto_.currency_id);
            
            if (!market_data) {
                ESP_LOGW(TAG, "No market data available for currency ID: %d", screensaver_crypto_.currency_id);
                // 即使没有市场数据，也要更新价格和涨跌幅为默认值
                if (screensaver_crypto_price_) {
                    char price_text[32];
                    snprintf(price_text, sizeof(price_text), "$%.2f", screensaver_crypto_.price);
                    lv_label_set_text(screensaver_crypto_price_, price_text);
                }
                
                if (screensaver_crypto_change_) {
                    char change_text[32];
                    snprintf(change_text, sizeof(change_text), "%.2f%%", screensaver_crypto_.change_24h);
                    lv_label_set_text(screensaver_crypto_change_, change_text);

                    
                    // 根据涨跌设置颜色
                    if (screensaver_crypto_.change_24h >= 0) {
                        lv_obj_set_style_text_color(screensaver_crypto_change_, current_wxt185_theme_.crypto_up_color, 0);
                    } else {
                        lv_obj_set_style_text_color(screensaver_crypto_change_, current_wxt185_theme_.crypto_down_color, 0);
                    }
                }
            } else {
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
            }
        } catch (const std::exception& e) {
            ESP_LOGE(TAG, "Exception occurred while updating screensaver market data: %s", e.what());
        } catch (...) {
            ESP_LOGE(TAG, "Unknown exception occurred while updating screensaver market data");
        }
    } else {
        ESP_LOGW(TAG, "BiJie coins service not initialized");
        // 即使币界服务未初始化，也要显示默认的虚拟币信息
        if (screensaver_crypto_price_) {
            char price_text[32];
            snprintf(price_text, sizeof(price_text), "$%.2f", screensaver_crypto_.price);
            lv_label_set_text(screensaver_crypto_price_, price_text);
        }
        
        if (screensaver_crypto_change_) {
            char change_text[32];
            snprintf(change_text, sizeof(change_text), "%.2f%%", screensaver_crypto_.change_24h);
            lv_label_set_text(screensaver_crypto_change_, change_text);
            
            // 根据涨跌设置颜色
            if (screensaver_crypto_.change_24h >= 0) {
                lv_obj_set_style_text_color(screensaver_crypto_change_, current_wxt185_theme_.crypto_up_color, 0);
            } else {
                lv_obj_set_style_text_color(screensaver_crypto_change_, current_wxt185_theme_.crypto_down_color, 0);
            }
        }
    }

    ESP_LOGI(TAG, "Screensaver content updated");
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
    
    while (retry_count <= max_retries) {
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

// 添加友元函数用于LVGL异步调用处理K线数据
static void process_kline_data_async(void* user_data) {
    // 检查参数有效性
    if (!user_data) {
        ESP_LOGE(TAG, "process_kline_data_async: user_data is null");
        return;
    }
    
    auto kline_data_ptr = static_cast<std::pair<WXT185Display*, std::vector<KLineData>*>*>(user_data);
    
    // 检查参数有效性
    if (!kline_data_ptr) {
        ESP_LOGE(TAG, "process_kline_data_async: kline_data_ptr is null");
        // 清理可能分配的内存
        delete static_cast<std::pair<WXT185Display*, std::vector<KLineData>*>*>(user_data);
        return;
    }
    
    WXT185Display* self = kline_data_ptr->first;
    std::vector<KLineData>* kline_data = kline_data_ptr->second;
    
    // 检查对象有效性
    if (!self || !kline_data) {
        ESP_LOGE(TAG, "process_kline_data_async: self or kline_data is null");
        delete kline_data;
        delete kline_data_ptr;
        return;
    }
    
    ESP_LOGI(TAG, "process_kline_data_async Received K-line data with %d points", (int)kline_data->size());
    
    try {
        // 注意：不再将K线数据存储到crypto_data_中，而是直接使用bijie_coins_服务中的数据
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
}

void WXT185Display::ConnectToBiJieCoins() {
    ESP_LOGI(TAG, "Connecting to BiJie coins service");
    if (!bijie_coins_) {
        ESP_LOGW(TAG, "BiJie coins service not initialized");
        return;
    }
    
    // 检查是否已经在连接过程中
    static bool is_connecting = false;
    if (is_connecting) {
        ESP_LOGW(TAG, "Already connecting to BiJie coins, skipping");
        return;
    }
    
    is_connecting = true;
    
    // 创建一个专门的任务来处理WebSocket连接，避免阻塞UI线程
    // 传递this指针作为参数，以便在任务中访问WXT185Display对象
    xTaskCreate([](void* param) {
        WXT185Display* self = static_cast<WXT185Display*>(param);
        
        // 确保在函数结束时重置连接状态
        struct ConnectGuard {
            bool& connecting_flag;
            ConnectGuard(bool& flag) : connecting_flag(flag) {}
            ~ConnectGuard() { connecting_flag = false; }
        } guard(is_connecting);
        
        // 检查对象有效性
        if (!self) {
            ESP_LOGE(TAG, "ConnectToBiJieCoins: self is null");
            vTaskDelete(nullptr);
            return;
        }
        
        try {
            // 等待网络就绪
            if (!self->WaitForNetworkReady()) {
                ESP_LOGE(TAG, "Network is not ready, aborting BiJie coins connection");
                vTaskDelete(nullptr);
                return;
            }
            
            // 检查bijie_coins是否仍然有效
            if (!self->bijie_coins_) {
                ESP_LOGE(TAG, "BiJie coins service no longer available");
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
            
            // 根据控制变量决定是否获取K线数据
            if (self->enable_kline_crypto_data_) {
                ESP_LOGI(TAG, "Getting K-line data for currency %d", self->current_crypto_data_.currency_id);
                // 获取K线数据用于图表显示
                self->bijie_coins_->GetKLineData(self->current_crypto_data_.currency_id, self->GetKLineTypeByIndex(self->selected_kline_frequency_), 30, [self](const std::vector<KLineData>& kline_data) {
                    // 检查参数有效性
                    if (!self) {
                        ESP_LOGE(TAG, "K-line data callback: self is null");
                        vTaskDelete(nullptr);
                        return;
                    }
                    
                    try {
                        // 直接在LVGL线程中更新UI，确保线程安全
                        self->DrawKLineChart();
                    } catch (const std::exception& e) {
                        ESP_LOGE(TAG, "Exception occurred in K-line data callback: %s", e.what());
                    } catch (...) {
                        ESP_LOGE(TAG, "Unknown exception occurred in K-line data callback");
                    }
                });
            }
            
            self->bijie_coins_connected_ = true;
            
            // 获取K线数据用于图表显示
            if (self->enable_kline_crypto_data_ && self->crypto_page_ && !lv_obj_has_flag(self->crypto_page_, LV_OBJ_FLAG_HIDDEN)) {
                ESP_LOGI(TAG, "Getting initial K-line data for currency %d", self->current_crypto_data_.currency_id);
                self->bijie_coins_->GetKLineData(self->current_crypto_data_.currency_id, 
                                                self->GetKLineTypeByIndex(self->selected_kline_frequency_), 
                                                30, 
                                                [self](const std::vector<KLineData>& kline_data) {
                    ESP_LOGI(TAG, "Received initial K-line data with %d points", kline_data.size());
                    // 更新图表显示
                    self->DrawKLineChart();
                });
            }
        } catch (const std::exception& e) {
            ESP_LOGE(TAG, "Exception occurred while connecting to BiJie coins: %s", e.what());
        } catch (...) {
            ESP_LOGE(TAG, "Unknown exception occurred while connecting to BiJie coins");
        }
        
        // 显式删除任务，防止任务返回导致系统错误
        vTaskDelete(nullptr);
    }, "bijie_coins_con", 4096, this, 5, nullptr);
}

void WXT185Display::UpdateCryptoDataFromBiJie() {
    ESP_LOGI(TAG, "Updating crypto data from BiJie service");
    if (!bijie_coins_ || !bijie_coins_connected_) {
        ESP_LOGW(TAG, "BiJie coins service not connected");
        return;
    }
    
    // 设置币界行情数据回调函数，用于实时更新当前显示的虚拟币数据
    bijie_coins_->SetMarketDataCallback([this](const CoinMarketData& market_data) {
        ESP_LOGI(TAG, "Received market data callback for currency ID: %d", market_data.currency_id);
        
        // 查找对应的虚拟币数据
        for (auto& crypto : crypto_data_) {
            if (crypto.currency_id == market_data.currency_id) {
                // 更新价格和24小时变化数据
                crypto.price = market_data.close;
                crypto.change_24h = market_data.change_24h;
                
                ESP_LOGI(TAG, "Updated %s: price=%.2f, change=%.2f%%", 
                         crypto.symbol.c_str(), crypto.price, crypto.change_24h);
                
                // 如果当前在虚拟币页面，更新显示
                if (current_page_index_ == PAGE_CRYPTO) {
                    UpdateCryptoData();
                }
                
                // 更新当前选中的货币数据（current_crypto_data_）
                if (market_data.currency_id == current_crypto_data_.currency_id) {
                    current_crypto_data_.price = market_data.close;
                    current_crypto_data_.change_24h = market_data.change_24h;
                    
                    // 如果是当前显示的货币，也更新顶部的价格和涨跌幅标签
                    if (crypto_price_label_) {
                        char price_text[32];
                        snprintf(price_text, sizeof(price_text), "%s $%.2f", 
                                 crypto.symbol.c_str(), market_data.close);
                        lv_label_set_text(crypto_price_label_, price_text);
                    }
                    
                    // 更新涨跌幅标签
                    if (crypto_change_label_) {
                        char change_text[32];
                        snprintf(change_text, sizeof(change_text), "%.2f%%", market_data.change_24h);
                        lv_label_set_text(crypto_change_label_, change_text);
                        
                        // 根据涨跌设置颜色
                        if (market_data.change_24h >= 0) {
                            lv_obj_set_style_text_color(crypto_change_label_, lv_color_hex(0x00FF00), 0);
                        } else {
                            lv_obj_set_style_text_color(crypto_change_label_, lv_color_hex(0xFF0000), 0);
                        }
                    }
                    
                    // 更新图表
                    DrawKLineChart();
                }
                break;
            }
        }
        
        // 如果屏保激活，也更新屏保显示内容
        if (screensaver_active_) {
            UpdateScreensaverContent();
        }
    });
    
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
        bijie_coins_->GetKLineData(current_crypto_data_.currency_id, GetKLineTypeByIndex(selected_kline_frequency_), 30, [this](const std::vector<KLineData>& kline_data) {
            try {
                ESP_LOGI(TAG, "Received K-line data with %d points", (int)kline_data.size());
                
                // 直接更新图表显示，不再存储到crypto_data_
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
uint32_t WXT185Display::GetKLineTypeByIndex(uint8_t index) {
    /*
    K线类型映射，与DrawKLineChart函数中的switch语句保持一致:
    case 0: // 1分钟 -> 13
    case 1: // 5分钟 -> 14
    case 2: // 15分钟 -> 1
    case 3: // 1小时 -> 2 (默认)
    case 4: // 2小时 -> 10
    case 5: // 4小时 -> 11
    case 6: // 1天 -> 3
    case 7: // 1周 -> 4
    case 8: // 1月 -> 5
    case 9: // 3月 -> 12
    */
    switch (index) {
        case 0: return 13; // 1分钟
        case 1: return 14; // 5分钟
        case 2: return 1;  // 15分钟
        case 3: return 2;  // 1小时
        case 4: return 10; // 2小时
        case 5: return 11; // 4小时
        case 6: return 3;  // 1天
        case 7: return 4;  // 1周
        case 8: return 5;  // 1月
        case 9: return 12; // 3月
        default: return 2; // 默认使用1小时
    }
}


// 通用 roller 样式设置函数
void WXT185Display::ApplyRollerStyle(lv_obj_t* roller, lv_style_t& style_roller_bg) {
    if (!roller) return;
    
    // 创建并配置样式
    //lv_style_init(&style_roller_bg);
    
    // 设置滚动背景颜色（使用主题中定义的roller背景颜色）
    lv_color_t bg_color = current_wxt185_theme_.roller_bg;
    lv_style_set_bg_color(&style_roller_bg, bg_color);
    
    // 设置背景透明度
    lv_style_set_bg_opa(&style_roller_bg, 200);
    
    // 设置边框
    lv_color_t border_color = current_wxt185_theme_.roller_border;
    lv_style_set_border_color(&style_roller_bg, border_color);
    lv_style_set_border_width(&style_roller_bg, 2);
    
    // 设置圆角
    lv_style_set_radius(&style_roller_bg, current_wxt185_theme_.roller_radius);
    
    // 将样式应用到 roller
    lv_obj_add_style(roller, &style_roller_bg, 0);
    
    // 设置文本颜色
    lv_color_t text_color = current_wxt185_theme_.roller_text;
    lv_obj_set_style_text_color(roller, text_color, 0);
    
    // 设置指示器颜色
    lv_color_t indicator_color = current_wxt185_theme_.roller_indicator;
    lv_obj_set_style_border_color(roller, indicator_color, LV_PART_SELECTED);
}