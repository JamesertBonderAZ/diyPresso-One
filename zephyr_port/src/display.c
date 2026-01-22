/*
 * Display interface for diyPresso
 * Ported to Zephyr RTOS
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <string.h>

#include "display.h"
#include "gpio_config.h"

LOG_MODULE_REGISTER(display, CONFIG_DIYPRESSO_LOG_LEVEL);

/* Configuration from device tree or Kconfig */
#define DISPLAY_I2C_DEV DT_LABEL(DT_BUS(DT_PATH(zephyr_user), i2c))
#define DISPLAY_I2C_ADDR DT_REG_ADDR(DT_PATH(zephyr_user, lcd))

/* PCF8574 pin definitions for LCD */
#define PCF_RS 0
#define PCF_RW 1
#define PCF_EN 2
#define PCF_BL 3
#define PCF_D4 4
#define PCF_D5 5
#define PCF_D6 6
#define PCF_D7 7

/* LCD commands */
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5X8DOTS 0x00

/* Static variables */
static const struct device *i2c_dev;
static display_error_t disp_error = DISPLAY_ERROR_NONE;
static bool backlight_state = true;

/* Forward declarations */
static int lcd_write_nibble(uint8_t nibble, bool rs);
static int lcd_write_byte(uint8_t data, bool rs);
static int lcd_command(uint8_t cmd);
static int lcd_write_char(char data);
static int lcd_init_sequence(void);

/**
 * @brief Initialize display
 */
int display_init(void)
{
    LOG_INF("Initializing display");

    /* Get I2C device */
    i2c_dev = DEVICE_DT_GET(DT_PATH(zephyr_user, lcd));
    if (!device_is_ready(i2c_dev)) {
        LOG_ERR("I2C device not ready");
        disp_error = DISPLAY_ERROR_INIT;
        return -ENODEV;
    }

    /* Initialize LCD */
    int ret = lcd_init_sequence();
    if (ret != 0) {
        LOG_ERR("LCD init sequence failed: %d", ret);
        disp_error = DISPLAY_ERROR_INIT;
        return ret;
    }

    LOG_INF("Display initialized successfully");
    disp_error = DISPLAY_ERROR_NONE;
    return 0;
}

/**
 * @brief Initialize LCD with proper sequence
 */
static int lcd_init_sequence(void)
{
    k_msleep(50);  /* Wait for more than 40ms after VCC rises to 4.5V */

    /* Send initial 0x03 sequence */
    lcd_write_nibble(0x03, false);
    k_msleep(5);  /* Wait for more than 4.1ms */
    
    lcd_write_nibble(0x03, false);
    k_usleep(150);  /* Wait for more than 100us */
    
    lcd_write_nibble(0x03, false);
    k_msleep(1);  /* Wait for more than 1ms */
    
    /* Set to 4-bit mode */
    lcd_write_nibble(0x02, false);
    k_msleep(1);

    /* Begin commands */
    lcd_command(LCD_FUNCTIONSET | LCD_2LINE | LCD_5X8DOTS | LCD_4BITMODE);
    k_msleep(1);
    
    lcd_command(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
    k_msleep(1);
    
    lcd_command(LCD_CLEARDISPLAY);
    k_msleep(2);
    
    lcd_command(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
    k_msleep(1);

    return 0;
}

/**
 * @brief Write a nibble to the LCD via I2C
 */
static int lcd_write_nibble(uint8_t nibble, bool rs)
{
    uint8_t data = 0;

    /* Set RS bit */
    if (rs) {
        data |= (1 << PCF_RS);
    }

    /* Set backlight */
    if (backlight_state) {
        data |= (1 << PCF_BL);
    }

    /* Set data bits (D4-D7) */
    if (nibble & 0x01) data |= (1 << PCF_D4);
    if (nibble & 0x02) data |= (1 << PCF_D5);
    if (nibble & 0x04) data |= (1 << PCF_D6);
    if (nibble & 0x08) data |= (1 << PCF_D7);

    /* Write with EN pulse */
    int ret = i2c_reg_write_byte(i2c_dev, DISPLAY_I2C_ADDR, 0, data);
    if (ret != 0) {
        return ret;
    }

    /* Pulse EN */
    data |= (1 << PCF_EN);  /* Enable HIGH */
    ret = i2c_reg_write_byte(i2c_dev, DISPLAY_I2C_ADDR, 0, data);
    if (ret != 0) {
        return ret;
    }

    data &= ~(1 << PCF_EN);  /* Enable LOW */
    ret = i2c_reg_write_byte(i2c_dev, DISPLAY_I2C_ADDR, 0, data);
    if (ret != 0) {
        return ret;
    }

    return 0;
}

/**
 * @brief Write a byte to the LCD
 */
static int lcd_write_byte(uint8_t data, bool rs)
{
    int ret;

    /* Write high nibble */
    ret = lcd_write_nibble((data >> 4) & 0x0F, rs);
    if (ret != 0) {
        return ret;
    }

    /* Write low nibble */
    ret = lcd_write_nibble(data & 0x0F, rs);
    if (ret != 0) {
        return ret;
    }

    return 0;
}

/**
 * @brief Send command to LCD
 */
static int lcd_command(uint8_t cmd)
{
    return lcd_write_byte(cmd, false);
}

/**
 * @brief Write character to LCD
 */
static int lcd_write_char(char data)
{
    return lcd_write_byte((uint8_t)data, true);
}

/**
 * @brief Print string at specific position
 */
int display_print(const char *str, int row, int col)
{
    if (!str || row < 0 || row >= DISPLAY_ROWS || col < 0 || col >= DISPLAY_COLS) {
        return -EINVAL;
    }

    /* Set cursor position */
    uint8_t pos = col;
    switch (row) {
        case 0: pos = 0x00; break;
        case 1: pos = 0x40; break;
        case 2: pos = 0x14; break;  /* 0x14 = 20 in decimal */
        case 3: pos = 0x54; break;  /* 0x54 = 20+64+4 = 84 in decimal */
        default: return -EINVAL;
    }

    int ret = lcd_command(LCD_SETDDRAMADDR | (pos + col));
    if (ret != 0) {
        disp_error = DISPLAY_ERROR_COMM;
        return ret;
    }

    /* Print string */
    for (int i = 0; str[i] != '\0' && i < (DISPLAY_COLS - col); i++) {
        ret = lcd_write_char(str[i]);
        if (ret != 0) {
            disp_error = DISPLAY_ERROR_COMM;
            return ret;
        }
    }

    disp_error = DISPLAY_ERROR_NONE;
    return 0;
}

/**
 * @brief Clear entire display
 */
int display_clear(void)
{
    int ret = lcd_command(LCD_CLEARDISPLAY);
    if (ret != 0) {
        disp_error = DISPLAY_ERROR_COMM;
        return ret;
    }
    k_msleep(2);  /* Clear command takes 1.52ms */
    disp_error = DISPLAY_ERROR_NONE;
    return 0;
}

/**
 * @brief Clear a specific row
 */
int display_clear_row(int row)
{
    if (row < 0 || row >= DISPLAY_ROWS) {
        return -EINVAL;
    }

    char spaces[DISPLAY_COLS + 1];
    memset(spaces, ' ', DISPLAY_COLS);
    spaces[DISPLAY_COLS] = '\0';

    return display_print(spaces, row, 0);
}

/**
 * @brief Set cursor position
 */
int display_set_cursor(int row, int col)
{
    if (row < 0 || row >= DISPLAY_ROWS || col < 0 || col >= DISPLAY_COLS) {
        return -EINVAL;
    }

    uint8_t pos = col;
    switch (row) {
        case 0: pos = 0x00; break;
        case 1: pos = 0x40; break;
        case 2: pos = 0x14; break;
        case 3: pos = 0x54; break;
        default: return -EINVAL;
    }

    int ret = lcd_command(LCD_SETDDRAMADDR | (pos + col));
    if (ret != 0) {
        disp_error = DISPLAY_ERROR_COMM;
        return ret;
    }

    disp_error = DISPLAY_ERROR_NONE;
    return 0;
}

/**
 * @brief Turn backlight on
 */
int display_backlight_on(void)
{
    backlight_state = true;
    disp_error = DISPLAY_ERROR_NONE;
    return 0;
}

/**
 * @brief Turn backlight off
 */
int display_backlight_off(void)
{
    backlight_state = false;
    disp_error = DISPLAY_ERROR_NONE;
    return 0;
}

/**
 * @brief Check if there's an error
 */
bool display_is_error(void)
{
    return disp_error != DISPLAY_ERROR_NONE;
}

/**
 * @brief Get error type
 */
display_error_t display_get_error(void)
{
    return disp_error;
}

/**
 * @brief Get error text description
 */
const char *display_get_error_text(void)
{
    switch (disp_error) {
        case DISPLAY_ERROR_NONE:
            return "No Error";
        case DISPLAY_ERROR_INIT:
            return "Initialization Error";
        case DISPLAY_ERROR_COMM:
            return "Communication Error";
        case DISPLAY_ERROR_BUSY:
            return "Busy Error";
        default:
            return "Unknown Error";
    }
}

/**
 * @brief Clear error state
 */
void display_clear_error(void)
{
    disp_error = DISPLAY_ERROR_NONE;
}