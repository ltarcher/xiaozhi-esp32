#include "wifi_board.h"
#include "codecs/no_audio_codec.h"
#include "display/lcd_display.h"
#include "display/wxt185_display.h"  // 添加新的显示类头文件
#include "system_reset.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "device_state_event.h"  // 添加设备状态事件头文件
#include <esp_err.h>
#include "esp_check.h"
#include <esp_log.h>
#include <driver/i2c_master.h>
#include <driver/ledc.h>
#include <wifi_station.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_st77916.h>
#include <esp_timer.h>
#include "esp_io_expander_tca9554.h"
#include <esp_lvgl_port.h>
#include "esp_lcd_touch.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_lcd_touch_cst816s.h"
#include "mcp_server.h"

#define TAG "waveshare_lcd_1_85"

#define LCD_OPCODE_WRITE_CMD        (0x02ULL)
#define LCD_OPCODE_READ_CMD         (0x0BULL)
#define LCD_OPCODE_WRITE_COLOR      (0x32ULL)

LV_FONT_DECLARE(font_puhui_16_4);
LV_FONT_DECLARE(font_awesome_16_4);

static const st77916_lcd_init_cmd_t vendor_specific_init_new[] = {
    {0xF0, (uint8_t []){0x28}, 1, 0},
    {0xF2, (uint8_t []){0x28}, 1, 0},
    {0x73, (uint8_t []){0xF0}, 1, 0},
    {0x7C, (uint8_t []){0xD1}, 1, 0},
    {0x83, (uint8_t []){0xE0}, 1, 0},
    {0x84, (uint8_t []){0x61}, 1, 0},
    {0xF2, (uint8_t []){0x82}, 1, 0},
    {0xF0, (uint8_t []){0x00}, 1, 0},
    {0xF0, (uint8_t []){0x01}, 1, 0},
    {0xF1, (uint8_t []){0x01}, 1, 0},
    {0xB0, (uint8_t []){0x56}, 1, 0},
    {0xB1, (uint8_t []){0x4D}, 1, 0},
    {0xB2, (uint8_t []){0x24}, 1, 0},
    {0xB4, (uint8_t []){0x87}, 1, 0},
    {0xB5, (uint8_t []){0x44}, 1, 0},
    {0xB6, (uint8_t []){0x8B}, 1, 0},
    {0xB7, (uint8_t []){0x40}, 1, 0},
    {0xB8, (uint8_t []){0x86}, 1, 0},
    {0xBA, (uint8_t []){0x00}, 1, 0},
    {0xBB, (uint8_t []){0x08}, 1, 0},
    {0xBC, (uint8_t []){0x08}, 1, 0},
    {0xBD, (uint8_t []){0x00}, 1, 0},
    {0xC0, (uint8_t []){0x80}, 1, 0},
    {0xC1, (uint8_t []){0x10}, 1, 0},
    {0xC2, (uint8_t []){0x37}, 1, 0},
    {0xC3, (uint8_t []){0x80}, 1, 0},
    {0xC4, (uint8_t []){0x10}, 1, 0},
    {0xC5, (uint8_t []){0x37}, 1, 0},
    {0xC6, (uint8_t []){0xA9}, 1, 0},
    {0xC7, (uint8_t []){0x41}, 1, 0},
    {0xC8, (uint8_t []){0x01}, 1, 0},
    {0xC9, (uint8_t []){0xA9}, 1, 0},
    {0xCA, (uint8_t []){0x41}, 1, 0},
    {0xCB, (uint8_t []){0x01}, 1, 0},
    {0xD0, (uint8_t []){0x91}, 1, 0},
    {0xD1, (uint8_t []){0x68}, 1, 0},
    {0xD2, (uint8_t []){0x68}, 1, 0},
    {0xF5, (uint8_t []){0x00, 0xA5}, 2, 0},
    {0xDD, (uint8_t []){0x4F}, 1, 0},
    {0xDE, (uint8_t []){0x4F}, 1, 0},
    {0xF1, (uint8_t []){0x10}, 1, 0},
    {0xF0, (uint8_t []){0x00}, 1, 0},
    {0xF0, (uint8_t []){0x02}, 1, 0},
    {0xE0, (uint8_t []){0xF0, 0x0A, 0x10, 0x09, 0x09, 0x36, 0x35, 0x33, 0x4A, 0x29, 0x15, 0x15, 0x2E, 0x34}, 14, 0},
    {0xE1, (uint8_t []){0xF0, 0x0A, 0x0F, 0x08, 0x08, 0x05, 0x34, 0x33, 0x4A, 0x39, 0x15, 0x15, 0x2D, 0x33}, 14, 0},
    {0xF0, (uint8_t []){0x10}, 1, 0},
    {0xF3, (uint8_t []){0x10}, 1, 0},
    {0xE0, (uint8_t []){0x07}, 1, 0},
    {0xE1, (uint8_t []){0x00}, 1, 0},
    {0xE2, (uint8_t []){0x00}, 1, 0},
    {0xE3, (uint8_t []){0x00}, 1, 0},
    {0xE4, (uint8_t []){0xE0}, 1, 0},
    {0xE5, (uint8_t []){0x06}, 1, 0},
    {0xE6, (uint8_t []){0x21}, 1, 0},
    {0xE7, (uint8_t []){0x01}, 1, 0},
    {0xE8, (uint8_t []){0x05}, 1, 0},
    {0xE9, (uint8_t []){0x02}, 1, 0},
    {0xEA, (uint8_t []){0xDA}, 1, 0},
    {0xEB, (uint8_t []){0x00}, 1, 0},
    {0xEC, (uint8_t []){0x00}, 1, 0},
    {0xED, (uint8_t []){0x0F}, 1, 0},
    {0xEE, (uint8_t []){0x00}, 1, 0},
    {0xEF, (uint8_t []){0x00}, 1, 0},
    {0xF8, (uint8_t []){0x00}, 1, 0},
    {0xF9, (uint8_t []){0x00}, 1, 0},
    {0xFA, (uint8_t []){0x00}, 1, 0},
    {0xFB, (uint8_t []){0x00}, 1, 0},
    {0xFC, (uint8_t []){0x00}, 1, 0},
    {0xFD, (uint8_t []){0x00}, 1, 0},
    {0xFE, (uint8_t []){0x00}, 1, 0},
    {0xFF, (uint8_t []){0x00}, 1, 0},
    {0x60, (uint8_t []){0x40}, 1, 0},
    {0x61, (uint8_t []){0x04}, 1, 0},
    {0x62, (uint8_t []){0x00}, 1, 0},
    {0x63, (uint8_t []){0x42}, 1, 0},
    {0x64, (uint8_t []){0xD9}, 1, 0},
    {0x65, (uint8_t []){0x00}, 1, 0},
    {0x66, (uint8_t []){0x00}, 1, 0},
    {0x67, (uint8_t []){0x00}, 1, 0},
    {0x68, (uint8_t []){0x00}, 1, 0},
    {0x69, (uint8_t []){0x00}, 1, 0},
    {0x6A, (uint8_t []){0x00}, 1, 0},
    {0x6B, (uint8_t []){0x00}, 1, 0},
    {0x70, (uint8_t []){0x40}, 1, 0},
    {0x71, (uint8_t []){0x03}, 1, 0},
    {0x72, (uint8_t []){0x00}, 1, 0},
    {0x73, (uint8_t []){0x42}, 1, 0},
    {0x74, (uint8_t []){0xD8}, 1, 0},
    {0x75, (uint8_t []){0x00}, 1, 0},
    {0x76, (uint8_t []){0x00}, 1, 0},
    {0x77, (uint8_t []){0x00}, 1, 0},
    {0x78, (uint8_t []){0x00}, 1, 0},
    {0x79, (uint8_t []){0x00}, 1, 0},
    {0x7A, (uint8_t []){0x00}, 1, 0},
    {0x7B, (uint8_t []){0x00}, 1, 0},
    {0x80, (uint8_t []){0x48}, 1, 0},
    {0x81, (uint8_t []){0x00}, 1, 0},
    {0x82, (uint8_t []){0x06}, 1, 0},
    {0x83, (uint8_t []){0x02}, 1, 0},
    {0x84, (uint8_t []){0xD6}, 1, 0},
    {0x85, (uint8_t []){0x04}, 1, 0},
    {0x86, (uint8_t []){0x00}, 1, 0},
    {0x87, (uint8_t []){0x00}, 1, 0},
    {0x88, (uint8_t []){0x48}, 1, 0},
    {0x89, (uint8_t []){0x00}, 1, 0},
    {0x8A, (uint8_t []){0x08}, 1, 0},
    {0x8B, (uint8_t []){0x02}, 1, 0},
    {0x8C, (uint8_t []){0xD8}, 1, 0},
    {0x8D, (uint8_t []){0x04}, 1, 0},
    {0x8E, (uint8_t []){0x00}, 1, 0},
    {0x8F, (uint8_t []){0x00}, 1, 0},
    {0x90, (uint8_t []){0x48}, 1, 0},
    {0x91, (uint8_t []){0x00}, 1, 0},
    {0x92, (uint8_t []){0x0A}, 1, 0},
    {0x93, (uint8_t []){0x02}, 1, 0},
    {0x94, (uint8_t []){0xDA}, 1, 0},
    {0x95, (uint8_t []){0x04}, 1, 0},
    {0x96, (uint8_t []){0x00}, 1, 0},
    {0x97, (uint8_t []){0x00}, 1, 0},
    {0x98, (uint8_t []){0x48}, 1, 0},
    {0x99, (uint8_t []){0x00}, 1, 0},
    {0x9A, (uint8_t []){0x0C}, 1, 0},
    {0x9B, (uint8_t []){0x02}, 1, 0},
    {0x9C, (uint8_t []){0xDC}, 1, 0},
    {0x9D, (uint8_t []){0x04}, 1, 0},
    {0x9E, (uint8_t []){0x00}, 1, 0},
    {0x9F, (uint8_t []){0x00}, 1, 0},
    {0xA0, (uint8_t []){0x48}, 1, 0},
    {0xA1, (uint8_t []){0x00}, 1, 0},
    {0xA2, (uint8_t []){0x05}, 1, 0},
    {0xA3, (uint8_t []){0x02}, 1, 0},
    {0xA4, (uint8_t []){0xD5}, 1, 0},
    {0xA5, (uint8_t []){0x04}, 1, 0},
    {0xA6, (uint8_t []){0x00}, 1, 0},
    {0xA7, (uint8_t []){0x00}, 1, 0},
    {0xA8, (uint8_t []){0x48}, 1, 0},
    {0xA9, (uint8_t []){0x00}, 1, 0},
    {0xAA, (uint8_t []){0x07}, 1, 0},
    {0xAB, (uint8_t []){0x02}, 1, 0},
    {0xAC, (uint8_t []){0xD7}, 1, 0},
    {0xAD, (uint8_t []){0x04}, 1, 0},
    {0xAE, (uint8_t []){0x00}, 1, 0},
    {0xAF, (uint8_t []){0x00}, 1, 0},
    {0xB0, (uint8_t []){0x48}, 1, 0},
    {0xB1, (uint8_t []){0x00}, 1, 0},
    {0xB2, (uint8_t []){0x09}, 1, 0},
    {0xB3, (uint8_t []){0x02}, 1, 0},
    {0xB4, (uint8_t []){0xD9}, 1, 0},
    {0xB5, (uint8_t []){0x04}, 1, 0},
    {0xB6, (uint8_t []){0x00}, 1, 0},
    {0xB7, (uint8_t []){0x00}, 1, 0},
    
    {0xB8, (uint8_t []){0x48}, 1, 0},
    {0xB9, (uint8_t []){0x00}, 1, 0},
    {0xBA, (uint8_t []){0x0B}, 1, 0},
    {0xBB, (uint8_t []){0x02}, 1, 0},
    {0xBC, (uint8_t []){0xDB}, 1, 0},
    {0xBD, (uint8_t []){0x04}, 1, 0},
    {0xBE, (uint8_t []){0x00}, 1, 0},
    {0xBF, (uint8_t []){0x00}, 1, 0},
    {0xC0, (uint8_t []){0x10}, 1, 0},
    {0xC1, (uint8_t []){0x47}, 1, 0},
    {0xC2, (uint8_t []){0x56}, 1, 0},
    {0xC3, (uint8_t []){0x65}, 1, 0},
    {0xC4, (uint8_t []){0x74}, 1, 0},
    {0xC5, (uint8_t []){0x88}, 1, 0},
    {0xC6, (uint8_t []){0x99}, 1, 0},
    {0xC7, (uint8_t []){0x01}, 1, 0},
    {0xC8, (uint8_t []){0xBB}, 1, 0},
    {0xC9, (uint8_t []){0xAA}, 1, 0},
    {0xD0, (uint8_t []){0x10}, 1, 0},
    {0xD1, (uint8_t []){0x47}, 1, 0},
    {0xD2, (uint8_t []){0x56}, 1, 0},
    {0xD3, (uint8_t []){0x65}, 1, 0},
    {0xD4, (uint8_t []){0x74}, 1, 0},
    {0xD5, (uint8_t []){0x88}, 1, 0},
    {0xD6, (uint8_t []){0x99}, 1, 0},
    {0xD7, (uint8_t []){0x01}, 1, 0},
    {0xD8, (uint8_t []){0xBB}, 1, 0},
    {0xD9, (uint8_t []){0xAA}, 1, 0},
    {0xF3, (uint8_t []){0x01}, 1, 0},
    {0xF0, (uint8_t []){0x00}, 1, 0},
    {0x21, (uint8_t []){0x00}, 1, 0},
    {0x11, (uint8_t []){0x00}, 1, 120},
    {0x29, (uint8_t []){0x00}, 1, 0},  
};

/**
CST816.h
*/
/**
 * @brief Create a new CST816S touch driver
 *
 * @note  The I2C communication should be initialized before use this function.
 *
 * @param io LCD panel IO handle, it should be created by `esp_lcd_new_panel_io_i2c()`
 * @param config Touch panel configuration
 * @param tp Touch panel handle
 * @return
 *      - ESP_OK: on success
 */
esp_err_t esp_lcd_touch_new_i2c_cst816s(const esp_lcd_panel_io_handle_t io, const esp_lcd_touch_config_t *config, esp_lcd_touch_handle_t *tp);

/**
 * @brief I2C address of the CST816S controller
 *
 */
#define ESP_LCD_TOUCH_IO_I2C_CST816S_ADDRESS    (0x15)

/**
 * @brief Touch IO configuration structure
 *
 */
#define ESP_LCD_TOUCH_IO_I2C_CST816_CONFIG()             \
    {                                                    \
        .dev_addr = ESP_LCD_TOUCH_IO_I2C_CST816S_ADDRESS, \
        .control_phase_bytes = 1,                        \
        .dc_bit_offset = 0,                              \
        .lcd_cmd_bits = 8,                              \
        .flags =                                         \
        {                                                \
            .disable_control_phase = 1,                  \
        }                                                \
    }
    

// I2C settings
#define I2C_Touch_SDA_IO            1               /*!< GPIO number used for I2C master data  */
#define I2C_Touch_SCL_IO            3               /*!< GPIO number used for I2C master clock */
#define I2C_Touch_INT_IO            4               /*!< GPIO number used for I2C master data  */
#define I2C_Touch_RST_IO            -1              /*!< GPIO number used for I2C master clock */
#define I2C_Touch_MASTER_NUM        1               /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_Touch_MASTER_FREQ_HZ    400000          /*!< I2C master clock frequency */

esp_io_expander_handle_t io_expander = NULL;

void InitCST816Touch(void);

///////////////////////////////////////////////////////////////////////////////////////
/**
 * CST816实现
 */
#define POINT_NUM_MAX       (1)

#define DATA_START_REG      (0x02)
#define CHIP_ID_REG         (0xA7)
#define AutoSleep_REG       (0xFE)

esp_lcd_touch_handle_t tp = NULL;

static esp_err_t read_data(esp_lcd_touch_handle_t tp);
static bool get_xy(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num);
static esp_err_t del(esp_lcd_touch_handle_t tp);

static esp_err_t i2c_read_bytes(esp_lcd_touch_handle_t tp, uint16_t reg, uint8_t *data, uint8_t len);
static esp_err_t i2c_write_bytes(esp_lcd_touch_handle_t tp, uint16_t reg, uint8_t* data, uint8_t len);

static esp_err_t reset(esp_lcd_touch_handle_t tp);
static esp_err_t read_id(esp_lcd_touch_handle_t tp);
static void AutoSleep(bool Sleep_State);

esp_err_t esp_lcd_touch_new_i2c_cst816(const esp_lcd_panel_io_handle_t io, const esp_lcd_touch_config_t *config, esp_lcd_touch_handle_t *tp)
{
    ESP_RETURN_ON_FALSE(io, ESP_ERR_INVALID_ARG, TAG, "Invalid io");
    ESP_RETURN_ON_FALSE(config, ESP_ERR_INVALID_ARG, TAG, "Invalid config");
    ESP_RETURN_ON_FALSE(tp, ESP_ERR_INVALID_ARG, TAG, "Invalid touch handle");

    /* Prepare main structure */
    esp_err_t ret = ESP_OK;
    esp_lcd_touch_handle_t cst816s = (esp_lcd_touch_handle_t)calloc(1, sizeof(esp_lcd_touch_t));
    ESP_GOTO_ON_FALSE(cst816s, ESP_ERR_NO_MEM, err, TAG, "Touch handle malloc failed");

    /* Communication interface */
    cst816s->io = io;
    /* Only supported callbacks are set */
    cst816s->read_data = read_data;
    cst816s->get_xy = get_xy;
    cst816s->del = del;
    /* Mutex */
    cst816s->data.lock.owner = portMUX_FREE_VAL;
    /* Save config */
    memcpy(&cst816s->config, config, sizeof(esp_lcd_touch_config_t));

    /* Prepare pin for touch interrupt */
    if (cst816s->config.int_gpio_num != GPIO_NUM_NC) {
        const gpio_config_t int_gpio_config = {
            .pin_bit_mask = BIT64(cst816s->config.int_gpio_num),
            .mode = GPIO_MODE_INPUT,
            .intr_type = (cst816s->config.levels.interrupt ? GPIO_INTR_POSEDGE : GPIO_INTR_NEGEDGE),
        };
        ESP_GOTO_ON_ERROR(gpio_config(&int_gpio_config), err, TAG, "GPIO intr config failed");

        /* Register interrupt callback */
        if (cst816s->config.interrupt_callback) {
            esp_lcd_touch_register_interrupt_callback(cst816s, cst816s->config.interrupt_callback);
        }
    }
    /* Prepare pin for touch controller reset */
    if (cst816s->config.rst_gpio_num != GPIO_NUM_NC) {
        const gpio_config_t rst_gpio_config = {
            .pin_bit_mask = BIT64(cst816s->config.rst_gpio_num),
            .mode = GPIO_MODE_OUTPUT,
        };
        ESP_GOTO_ON_ERROR(gpio_config(&rst_gpio_config), err, TAG, "GPIO reset config failed");
    }
    /* Reset controller */
    ESP_GOTO_ON_ERROR(reset(cst816s), err, TAG, "Reset failed");

    /* Read product id */
    ESP_GOTO_ON_ERROR(read_id(cst816s), err, TAG, "Read version failed");
    *tp = cst816s;
    AutoSleep(false);
    return ESP_OK;
err:
    if (cst816s) {
        del(cst816s);
    }
    ESP_LOGE(TAG, "Initialization failed!");
    return ret;
}

static esp_err_t read_data(esp_lcd_touch_handle_t tp)
{
    typedef struct {
        uint8_t num;
        uint8_t x_h : 4;
        uint8_t : 4;
        uint8_t x_l;
        uint8_t y_h : 4;
        uint8_t : 4;
        uint8_t y_l;
    } data_t;

    data_t point;
    ESP_RETURN_ON_ERROR(i2c_read_bytes(tp, DATA_START_REG, (uint8_t *)&point, sizeof(data_t)), TAG, "I2C read failed");

    portENTER_CRITICAL(&tp->data.lock);
    point.num = (point.num > POINT_NUM_MAX ? POINT_NUM_MAX : point.num);
    tp->data.points = point.num;
    /* Fill all coordinates */
    for (int i = 0; i < point.num; i++) {
        tp->data.coords[i].x = point.x_h << 8 | point.x_l;
        tp->data.coords[i].y = point.y_h << 8 | point.y_l;
    }
    portEXIT_CRITICAL(&tp->data.lock);

    return ESP_OK;
}

static bool get_xy(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
    portENTER_CRITICAL(&tp->data.lock);
    /* Count of points */
    *point_num = (tp->data.points > max_point_num ? max_point_num : tp->data.points);
    for (size_t i = 0; i < *point_num; i++) {
        x[i] = tp->data.coords[i].x;
        y[i] = tp->data.coords[i].y;

        if (strength) {
            strength[i] = tp->data.coords[i].strength;
        }
    }
    /* Invalidate */
    tp->data.points = 0;
    portEXIT_CRITICAL(&tp->data.lock);

    return (*point_num > 0);
}

static esp_err_t del(esp_lcd_touch_handle_t tp)
{
    /* Reset GPIO pin settings */
    if (tp->config.int_gpio_num != GPIO_NUM_NC) {
        gpio_reset_pin(tp->config.int_gpio_num);
        if (tp->config.interrupt_callback) {
            gpio_isr_handler_remove(tp->config.int_gpio_num);
        }
    }
    if (tp->config.rst_gpio_num != GPIO_NUM_NC) {
        gpio_reset_pin(tp->config.rst_gpio_num);
    }
    /* Release memory */
    free(tp);

    return ESP_OK;
}

static esp_err_t reset(esp_lcd_touch_handle_t tp)
{
    esp_err_t ret;
    ret = esp_io_expander_set_dir(io_expander, IO_EXPANDER_PIN_NUM_1 , IO_EXPANDER_OUTPUT);                 // 设置引脚 EXIO0 和 EXIO1 模式为输出
    ESP_ERROR_CHECK(ret);
    ret = esp_io_expander_set_level(io_expander, IO_EXPANDER_PIN_NUM_1 , 0);                                // 复位 LCD 
    ESP_ERROR_CHECK(ret);
    vTaskDelay(pdMS_TO_TICKS(100));
    ret = esp_io_expander_set_level(io_expander, IO_EXPANDER_PIN_NUM_1 , 1);                                // 复位 LCD 
    ESP_ERROR_CHECK(ret);
    vTaskDelay(pdMS_TO_TICKS(50));

    return ESP_OK;
}

static esp_err_t read_id(esp_lcd_touch_handle_t tp)
{
    uint8_t id;
    ESP_RETURN_ON_ERROR(i2c_read_bytes(tp, CHIP_ID_REG, &id, 1), TAG, "I2C read failed");
    ESP_LOGI(TAG, "IC id: %d", id);
    return ESP_OK;
}
/*!
    @brief  Fall asleep automatically
*/
static void AutoSleep(bool Sleep_State) {
    uint8_t Sleep_State_Set = (uint8_t)(!Sleep_State);
    i2c_write_bytes(tp, AutoSleep_REG, &Sleep_State_Set, 1);
}

static esp_err_t i2c_read_bytes(esp_lcd_touch_handle_t tp, uint16_t reg, uint8_t *data, uint8_t len)
{
    ESP_RETURN_ON_FALSE(data, ESP_ERR_INVALID_ARG, TAG, "Invalid data");

    return esp_lcd_panel_io_rx_param(tp->io, reg, data, len);
}

static esp_err_t i2c_write_bytes(esp_lcd_touch_handle_t tp, uint16_t reg, uint8_t* data, uint8_t len)
{
    assert(tp != NULL);

    // *INDENT-OFF*
    /* Write data */
    return esp_lcd_panel_io_tx_param(tp->io, reg, data, len);
    // *INDENT-ON*
}

#if 0
/**
 * @brief i2c master initialization
 */
esp_err_t Touch_I2C_Init(void)
{
    int i2c_master_port = I2C_Touch_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_Touch_SDA_IO,
        .scl_io_num = I2C_Touch_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_Touch_MASTER_FREQ_HZ,
    };

    i2c_param_config((i2c_port_t)i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}
#endif
#if 0
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void InitCST816Touch(void)
{
    
    ESP_ERROR_CHECK(Touch_I2C_Init());
    ESP_LOGI(TAG, "I2C initialized successfully");
/********************* Touch *********************/

    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_CST816_CONFIG();
    ESP_LOGI(TAG, "Initialize touch IO (I2C)");
    /* Touch IO handle */
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)I2C_Touch_MASTER_NUM, &tp_io_config, &tp_io_handle));
    esp_lcd_touch_config_t tp_cfg = {
        .x_max = DISPLAY_WIDTH,
        .y_max = DISPLAY_HEIGHT,
        .rst_gpio_num = (gpio_num_t)I2C_Touch_RST_IO,
        .int_gpio_num = (gpio_num_t)I2C_Touch_INT_IO,
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };
    /* Initialize touch */
    ESP_LOGI(TAG, "Initialize touch controller CST816");
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_cst816(tp_io_handle, &tp_cfg, &tp));
}
#endif


class CustomBoard : public WifiBoard {
private:
    i2c_master_bus_handle_t i2c_bus_;
    i2c_master_bus_handle_t tp_i2c_bus;
    esp_io_expander_handle_t io_expander;
    WXT185Display* display_;  // 修改为使用新的显示类
    button_handle_t boot_btn, pwr_btn;
    button_driver_t* boot_btn_driver_ = nullptr;
    button_driver_t* pwr_btn_driver_ = nullptr;
    static CustomBoard* instance_;


    void InitializeI2c() {
        // Initialize I2C peripheral
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = (i2c_port_t)0,
            .sda_io_num = I2C_SDA_IO,
            .scl_io_num = I2C_SCL_IO,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,  // 增加抗干扰能力
            .flags = {
                .enable_internal_pullup = 1,  // 启用内部上拉电阻
            },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &i2c_bus_));
    }
    
    void InitializeTca9554(void) {
        esp_err_t ret = esp_io_expander_new_i2c_tca9554(i2c_bus_, I2C_ADDRESS, &io_expander);
        if(ret != ESP_OK)
            ESP_LOGE(TAG, "TCA9554 create returned error");        

        // uint32_t input_level_mask = 0;
        // ret = esp_io_expander_set_dir(io_expander, IO_EXPANDER_PIN_NUM_0 | IO_EXPANDER_PIN_NUM_1, IO_EXPANDER_INPUT);               // 设置引脚 EXIO0 和 EXIO1 模式为输入 
        // ret = esp_io_expander_get_level(io_expander, IO_EXPANDER_PIN_NUM_0 | IO_EXPANDER_PIN_NUM_1, &input_level_mask);             // 获取引脚 EXIO0 和 EXIO1 的电平状态,存放在 input_level_mask 中

        // ret = esp_io_expander_set_dir(io_expander, IO_EXPANDER_PIN_NUM_2 | IO_EXPANDER_PIN_NUM_3, IO_EXPANDER_OUTPUT);              // 设置引脚 EXIO2 和 EXIO3 模式为输出
        // ret = esp_io_expander_set_level(io_expander, IO_EXPANDER_PIN_NUM_2 | IO_EXPANDER_PIN_NUM_3, 1);                             // 将引脚电平设置为 1
        // ret = esp_io_expander_print_state(io_expander);                                                                             // 打印引脚状态

        ret = esp_io_expander_set_dir(io_expander, IO_EXPANDER_PIN_NUM_0 | IO_EXPANDER_PIN_NUM_1 , IO_EXPANDER_OUTPUT);                 // 设置引脚 EXIO0 和 EXIO1 模式为输出
        ESP_ERROR_CHECK(ret);
        ret = esp_io_expander_set_level(io_expander, IO_EXPANDER_PIN_NUM_0 | IO_EXPANDER_PIN_NUM_1 , 1);                                // 复位 LCD 
        ESP_ERROR_CHECK(ret);
        vTaskDelay(pdMS_TO_TICKS(300));
        ret = esp_io_expander_set_level(io_expander, IO_EXPANDER_PIN_NUM_0 | IO_EXPANDER_PIN_NUM_1 , 0);                                // 复位 LCD 
        ESP_ERROR_CHECK(ret);
        vTaskDelay(pdMS_TO_TICKS(300));
        ret = esp_io_expander_set_level(io_expander, IO_EXPANDER_PIN_NUM_0 | IO_EXPANDER_PIN_NUM_1 , 1);                                // 复位 LCD
        ESP_ERROR_CHECK(ret);
    }

    void InitializeSpi() {
        ESP_LOGI(TAG, "Initialize QSPI bus");

        const spi_bus_config_t bus_config = TAIJIPI_ST77916_PANEL_BUS_QSPI_CONFIG(QSPI_PIN_NUM_LCD_PCLK,
                                                                        QSPI_PIN_NUM_LCD_DATA0,
                                                                        QSPI_PIN_NUM_LCD_DATA1,
                                                                        QSPI_PIN_NUM_LCD_DATA2,
                                                                        QSPI_PIN_NUM_LCD_DATA3,
                                                                        QSPI_LCD_H_RES * 80 * sizeof(uint16_t));
        ESP_ERROR_CHECK(spi_bus_initialize(QSPI_LCD_HOST, &bus_config, SPI_DMA_CH_AUTO));
    }

    void Initializest77916Display() {

        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;

        ESP_LOGI(TAG, "Install panel IO");
        
        esp_lcd_panel_io_spi_config_t io_config = {
            .cs_gpio_num = QSPI_PIN_NUM_LCD_CS,               
            .dc_gpio_num = -1,                  
            .spi_mode = 0,                     
            .pclk_hz = 3 * 1000 * 1000,      
            .trans_queue_depth = 10,            
            .on_color_trans_done = NULL,                            
            .user_ctx = NULL,                   
            .lcd_cmd_bits = 32,                 
            .lcd_param_bits = 8,                
            .flags = {                          
            .dc_low_on_data = 0,            
            .octal_mode = 0,                
            .quad_mode = 1,                 
            .sio_mode = 0,                  
            .lsb_first = 0,                 
            .cs_high_active = 0,            
            },                                  
        };
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)QSPI_LCD_HOST, &io_config, &panel_io));

        ESP_LOGI(TAG, "Install ST77916 panel driver");
        
        st77916_vendor_config_t vendor_config = {
            .flags = {
                .use_qspi_interface = 1,
            },
        };
        
        printf("-------------------------------------- Version selection -------------------------------------- \r\n");
        esp_err_t ret;
        int lcd_cmd = 0x04;
        uint8_t register_data[4]; 
        size_t param_size = sizeof(register_data);
        lcd_cmd &= 0xff;
        lcd_cmd <<= 8;
        lcd_cmd |= LCD_OPCODE_READ_CMD << 24;  // Use the read opcode instead of write
        ret = esp_lcd_panel_io_rx_param(panel_io, lcd_cmd, register_data, param_size); 
        if (ret == ESP_OK) {
            printf("Register 0x04 data: %02x %02x %02x %02x\n", register_data[0], register_data[1], register_data[2], register_data[3]);
        } else {
            printf("Failed to read register 0x04, error code: %d\n", ret);
        } 
        // panel_io_spi_del(io_handle);
        io_config.pclk_hz = 80 * 1000 * 1000;
        if (esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)QSPI_LCD_HOST, &io_config, &panel_io) != ESP_OK){
            printf("Failed to set LCD communication parameters -- SPI\r\n");
            return ;
        }
        printf("LCD communication parameters are set successfully -- SPI\r\n");
        
        // Check register values and configure accordingly
        if (register_data[0] == 0x00 && register_data[1] == 0x7F && register_data[2] == 0x7F && register_data[3] == 0x7F) {
            // Handle the case where the register data matches this pattern
            printf("Vendor-specific initialization for case 1.\n");
        }
        else if (register_data[0] == 0x00 && register_data[1] == 0x02 && register_data[2] == 0x7F && register_data[3] == 0x7F) {
            // Provide vendor-specific initialization commands if register data matches this pattern
            vendor_config.init_cmds = vendor_specific_init_new;
            vendor_config.init_cmds_size = sizeof(vendor_specific_init_new) / sizeof(st77916_lcd_init_cmd_t);
            printf("Vendor-specific initialization for case 2.\n");
        }
        printf("------------------------------------- End of version selection------------------------------------- \r\n");
 
        const esp_lcd_panel_dev_config_t panel_config = {
            .reset_gpio_num = QSPI_PIN_NUM_LCD_RST,
            .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,     // Implemented by LCD command `36h`
            .bits_per_pixel = QSPI_LCD_BIT_PER_PIXEL,    // Implemented by LCD command `3Ah` (16/18)
            .vendor_config = &vendor_config,
        };
        ESP_ERROR_CHECK(esp_lcd_new_panel_st77916(panel_io, &panel_config, &panel));

        esp_lcd_panel_reset(panel);
        esp_lcd_panel_init(panel);
        esp_lcd_panel_disp_on_off(panel, true);
        esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);
        esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);

        // 使用新的显示类替换原有的SpiLcdDisplay
        display_ = new WXT185Display(panel_io, panel,
                                    DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY,
                                    {
                                        .text_font = &font_puhui_16_4,
                                        .icon_font = &font_awesome_16_4,
                                        .emoji_font = font_emoji_64_init(),
                                    });
    }
 
#if CONFIG_ESP32_S3_TOUCH_LCD_185_WITH_TOUCH
    void InitializeI2cTouch() {
        // Create a separate I2C bus for touch controller
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = TP_PORT,
            .sda_io_num = TP_PIN_NUM_SDA,
            .scl_io_num = TP_PIN_NUM_SCL,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .flags = {
                .enable_internal_pullup = 1,
            },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &tp_i2c_bus));
    }

    void InitializeTouch()
    {
        ESP_LOGI(TAG, "Initializing touch controller...");
        ESP_LOGI(TAG, "Touch I2C config - SDA: %d, SCL: %d, Port: %d, Address: 0x%02X", 
                 TP_PIN_NUM_SDA, TP_PIN_NUM_SCL, TP_PORT, ESP_LCD_TOUCH_IO_I2C_CST816S_ADDRESS);
        
        esp_lcd_touch_handle_t tp;
        esp_lcd_touch_config_t tp_cfg = {
            .x_max = DISPLAY_WIDTH,
            .y_max = DISPLAY_HEIGHT,
            .rst_gpio_num = TP_PIN_NUM_RST,
            .int_gpio_num = TP_PIN_NUM_INT,
            .levels = {
                .reset = 0,
                .interrupt = 0,
            },
            .flags = {
                .swap_xy = 0,
                .mirror_x = 0,
                .mirror_y = 0,
            },
        };
        
        esp_lcd_panel_io_handle_t tp_io_handle = NULL;
        esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_CST816S_CONFIG();
        // 降低I2C速度以提高稳定性
        tp_io_config.scl_speed_hz = 100 * 1000;
        
        ESP_LOGI(TAG, "Probing for CST816S touch controller at address 0x%02X with 100kHz I2C speed", 
                 ESP_LCD_TOUCH_IO_I2C_CST816S_ADDRESS);
        
        // 添加I2C设备探测，确保触摸控制器存在
        esp_err_t ret = i2c_master_probe(tp_i2c_bus, ESP_LCD_TOUCH_IO_I2C_CST816S_ADDRESS, 2000);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "CST816S touch controller not found on I2C bus at address 0x%02X, error: %s", 
                     ESP_LCD_TOUCH_IO_I2C_CST816S_ADDRESS, esp_err_to_name(ret));
            
            // 尝试使用400kHz再探测一次
            ESP_LOGI(TAG, "Retrying with 400kHz I2C speed");
            tp_io_config.scl_speed_hz = 400 * 1000;
            ret = i2c_master_probe(tp_i2c_bus, ESP_LCD_TOUCH_IO_I2C_CST816S_ADDRESS, 2000);
            if (ret != ESP_OK) {
                ESP_LOGW(TAG, "Retry with 400kHz I2C speed also failed, error: %s", esp_err_to_name(ret));
                
                // 尝试使用主显示屏I2C总线
                ESP_LOGI(TAG, "Trying main I2C bus for touch controller");
                ESP_LOGI(TAG, "Main I2C config - SDA: %d, SCL: %d", I2C_SDA_IO, I2C_SCL_IO);
                
                // 先释放之前创建的I2C总线
                i2c_del_master_bus(tp_i2c_bus);
                
                // 使用主I2C总线重新探测
                tp_io_config.scl_speed_hz = 100 * 1000;
                ret = i2c_master_probe(i2c_bus_, ESP_LCD_TOUCH_IO_I2C_CST816S_ADDRESS, 2000);
                if (ret != ESP_OK) {
                    ESP_LOGW(TAG, "CST816S touch controller not found on main I2C bus at address 0x%02X with 100kHz, error: %s", 
                             ESP_LCD_TOUCH_IO_I2C_CST816S_ADDRESS, esp_err_to_name(ret));
                    
                    tp_io_config.scl_speed_hz = 400 * 1000;
                    ret = i2c_master_probe(i2c_bus_, ESP_LCD_TOUCH_IO_I2C_CST816S_ADDRESS, 2000);
                    if (ret != ESP_OK) {
                        ESP_LOGW(TAG, "CST816S touch controller not found on main I2C bus at address 0x%02X with 400kHz, error: %s", 
                                 ESP_LCD_TOUCH_IO_I2C_CST816S_ADDRESS, esp_err_to_name(ret));
                        ESP_LOGW(TAG, "Touch controller initialization failed, skipping touch functionality");
                        return;
                    } else {
                        ESP_LOGI(TAG, "Successfully detected touch controller on main I2C bus with 400kHz");
                        // 使用主I2C总线
                        ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus_, &tp_io_config, &tp_io_handle));
                    }
                } else {
                    ESP_LOGI(TAG, "Successfully detected touch controller on main I2C bus with 100kHz");
                    // 使用主I2C总线
                    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus_, &tp_io_config, &tp_io_handle));
                }
            } else {
                ESP_LOGI(TAG, "Successfully detected touch controller with 400kHz I2C speed on separate bus");
                ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(tp_i2c_bus, &tp_io_config, &tp_io_handle));
            }
        } else {
            ESP_LOGI(TAG, "Successfully detected touch controller with 100kHz I2C speed on separate bus");
            ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(tp_i2c_bus, &tp_io_config, &tp_io_handle));
        }
        
        ESP_LOGI(TAG, "Initialize touch controller CST816S");
        
        // 添加错误检查，避免系统崩溃
        ret = esp_lcd_touch_new_i2c_cst816s(tp_io_handle, &tp_cfg, &tp);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize CST816S touch controller: %s", esp_err_to_name(ret));
            return;
        }
        
        const lvgl_port_touch_cfg_t touch_cfg = {
            .disp = lv_display_get_default(), 
            .handle = tp,
        };
        lvgl_port_add_touch(&touch_cfg);
        ESP_LOGI(TAG, "Touch panel initialized successfully");
    }
#endif

    void InitializeButtonsCustom() {
        gpio_reset_pin(BOOT_BUTTON_GPIO);                                     
        gpio_set_direction(BOOT_BUTTON_GPIO, GPIO_MODE_INPUT);   
        gpio_reset_pin(PWR_BUTTON_GPIO);                                     
        gpio_set_direction(PWR_BUTTON_GPIO, GPIO_MODE_INPUT);   
        gpio_reset_pin(PWR_Control_PIN);                                     
        gpio_set_direction(PWR_Control_PIN, GPIO_MODE_OUTPUT);    
        // gpio_set_level(PWR_Control_PIN, false);
        gpio_set_level(PWR_Control_PIN, true); 
    }
    void InitializeButtons() {
        instance_ = this;
        InitializeButtonsCustom();

        // Boot Button
        button_config_t boot_btn_config = {
            .long_press_time = 2000,
            .short_press_time = 0
        };
        boot_btn_driver_ = (button_driver_t*)calloc(1, sizeof(button_driver_t));
        boot_btn_driver_->enable_power_save = false;
        boot_btn_driver_->get_key_level = [](button_driver_t *button_driver) -> uint8_t {
            return !gpio_get_level(BOOT_BUTTON_GPIO);
        };
        ESP_ERROR_CHECK(iot_button_create(&boot_btn_config, boot_btn_driver_, &boot_btn));
        iot_button_register_cb(boot_btn, BUTTON_SINGLE_CLICK, nullptr, [](void* button_handle, void* usr_data) {
            auto self = static_cast<CustomBoard*>(usr_data);
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {
                self->ResetWifiConfiguration();
            }
            app.ToggleChatState();
        }, this);

        // Power Button
        button_config_t pwr_btn_config = {
            .long_press_time = 5000,
            .short_press_time = 0
        };
        pwr_btn_driver_ = (button_driver_t*)calloc(1, sizeof(button_driver_t));
        pwr_btn_driver_->enable_power_save = false;
        pwr_btn_driver_->get_key_level = [](button_driver_t *button_driver) -> uint8_t {
            return !gpio_get_level(PWR_BUTTON_GPIO);
        };
        ESP_ERROR_CHECK(iot_button_create(&pwr_btn_config, pwr_btn_driver_, &pwr_btn));
        iot_button_register_cb(pwr_btn, BUTTON_LONG_PRESS_START, nullptr, [](void* button_handle, void* usr_data) {
            auto self = static_cast<CustomBoard*>(usr_data);
            if(self->GetBacklight()->brightness() > 0) {
                self->GetBacklight()->SetBrightness(0);
                gpio_set_level(PWR_Control_PIN, false);
            }
            else {
                self->GetBacklight()->RestoreBrightness();
                gpio_set_level(PWR_Control_PIN, true);
            }
        }, this);
    }

    void InitializeTools() {
        // 初始化工具
        auto& mcp_server = McpServer::GetInstance();
        // 定义设备的属性
        mcp_server.AddTool("self.ui.uppage", "上一页", PropertyList(), [this](const PropertyList& properties) -> ReturnValue {
            ESP_LOGI(TAG, "self.ui.uppage");
            display_->SwitchToPage(display_->current_page_index_ - 1 > PAGE_CHAT ? display_->current_page_index_ - 1 : 0);
            return true;
        });

        mcp_server.AddTool("self.ui.downpage", "下一页", PropertyList(), [this](const PropertyList& properties) -> ReturnValue {
            ESP_LOGI(TAG, "self.ui.downpage");
            display_->SwitchToPage(display_->current_page_index_ + 1 < MAX_PAGE_INDEX ? display_->current_page_index_ + 1 : MAX_PAGE_INDEX - 1);
            return true;
        });

    }

public:
    CustomBoard() {   
        InitializeI2c();
        InitializeI2cTouch();
        InitializeTca9554();
        InitializeSpi();
        Initializest77916Display();
        InitializeButtons();
        
#if CONFIG_ESP32_S3_TOUCH_LCD_185_WITH_TOUCH
        // Only initialize touch panel if the board has touch capability
        InitializeTouch();
#endif
        InitializeTools();
        GetBacklight()->RestoreBrightness();
        
        // 注册设备状态改变事件监听器
        DeviceStateEventManager::GetInstance().RegisterStateChangeCallback(
            [this](DeviceState previous_state, DeviceState current_state) {
                if (display_) {
                    display_->OnDeviceStateChanged(static_cast<int>(previous_state), static_cast<int>(current_state));
                }
            }
        );
    }

    virtual AudioCodec* GetAudioCodec() override {
        static NoAudioCodecSimplex audio_codec(AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK, AUDIO_I2S_SPK_GPIO_DOUT, I2S_STD_SLOT_BOTH, AUDIO_I2S_MIC_GPIO_SCK, AUDIO_I2S_MIC_GPIO_WS, AUDIO_I2S_MIC_GPIO_DIN, I2S_STD_SLOT_RIGHT); // I2S_STD_SLOT_LEFT / I2S_STD_SLOT_RIGHT / I2S_STD_SLOT_BOTH

        return &audio_codec;
    }

    virtual Display* GetDisplay() override {
        return display_;
    }
    
    virtual Backlight* GetBacklight() override {
        static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
        return &backlight;
    }
};

DECLARE_BOARD(CustomBoard);

CustomBoard* CustomBoard::instance_ = nullptr;