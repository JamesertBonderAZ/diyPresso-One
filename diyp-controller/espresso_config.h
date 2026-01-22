/*
 * Copyright (c) 2024 Espresso Controller Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ESPRESSO_CONFIG_H_
#define ESPRESSO_CONFIG_H_

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/i2c.h>

/* Pin assignments for STM Nucleo MB1180 Rev C */
#define PIN_BREW_SWITCH DT_GPIO_PIN(DT_ALIAS(brew_switch), gpios)
#define PIN_SSR_PUMP    DT_GPIO_PIN(DT_ALIAS(ssr_pump), gpios)
#define PIN_SSR_HEATER  DT_GPIO_PIN(DT_ALIAS(ssr_heater), gpios)

#define PIN_HX711_CLK   DT_GPIO_PIN(DT_ALIAS(hx711_clk), gpios)
#define PIN_HX711_DAT   DT_GPIO_PIN(DT_ALIAS(hx711_dat), gpios)

#define PIN_THERM_RDY   DT_GPIO_PIN(DT_ALIAS(therm_rdy), gpios)
#define PIN_THERM_CS    DT_GPIO_PIN(DT_ALIAS(therm_cs), gpios)
#define PIN_THERM_MOSI  DT_GPIO_PIN(DT_ALIAS(therm_mosi), gpios)
#define PIN_THERM_SCLK  DT_GPIO_PIN(DT_ALIAS(therm_sclk), gpios)
#define PIN_THERM_MISO  DT_GPIO_PIN(DT_ALIAS(therm_miso), gpios)

#define PIN_LCD_SDA     DT_GPIO_PIN(DT_ALIAS(lcd_sda), gpios)
#define PIN_LCD_SCL     DT_GPIO_PIN(DT_ALIAS(lcd_scl), gpios)

#define PIN_ENC_A       DT_GPIO_PIN(DT_ALIAS(enc_a), gpios)
#define PIN_ENC_B       DT_GPIO_PIN(DT_ALIAS(enc_b), gpios)
#define PIN_ENC_S       DT_GPIO_PIN(DT_ALIAS(enc_s), gpios)

/* Device definitions */
#define BREW_SWITCH_DEVICE    DT_LABEL(DT_ALIAS(brew_switch))
#define SSR_PUMP_DEVICE       DT_LABEL(DT_ALIAS(ssr_pump))
#define SSR_HEATER_DEVICE     DT_LABEL(DT_ALIAS(ssr_heater))

#define HX711_CLK_DEVICE      DT_LABEL(DT_ALIAS(hx711_clk))
#define HX711_DAT_DEVICE      DT_LABEL(DT_ALIAS(hx711_dat))

#define THERM_RDY_DEVICE      DT_LABEL(DT_ALIAS(therm_rdy))
#define THERM_CS_DEVICE       DT_LABEL(DT_ALIAS(therm_cs))
#define THERM_SPI_DEV         DT_LABEL(DT_NODELABEL(spi1))  /* Assuming SPI1 */

#define LCD_I2C_DEV           DT_LABEL(DT_NODELABEL(i2c1))  /* Assuming I2C1 */

#define ENC_A_DEVICE          DT_LABEL(DT_ALIAS(enc_a))
#define ENC_B_DEVICE          DT_LABEL(DT_ALIAS(enc_b))
#define ENC_S_DEVICE          DT_LABEL(DT_ALIAS(enc_s))

/* System constants */
#define LCD_ADDR              0x27
#define LCD_COLS              20
#define LCD_ROWS              4

/* Timing constants */
#define TEMP_READ_INTERVAL    200   /* ms */
#define DISPLAY_UPDATE_INTERVAL 500 /* ms */
#define SCALE_READ_INTERVAL   100   /* ms */
#define MAIN_LOOP_INTERVAL    50    /* ms */

/* Temperature control constants */
#define DEFAULT_SETPOINT      95.0f
#define TEMP_DEADBAND         0.5f
#define TEMP_HYSTERESIS       0.2f

/* PID constants */
#define KP                    30.0f
#define KI                    0.2f
#define KD                    100.0f

/* Brewing parameters */
#define DEFAULT_BREW_TIME     25000  /* ms (25 seconds) */
#define DEFAULT_TARGET_WEIGHT 36.0f  /* grams */

#endif /* ESPRESSO_CONFIG_H_ */