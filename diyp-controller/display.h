/*
 * Copyright (c) 2024 Espresso Controller Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>

#include "espresso_control.h"

/* LCD commands */
#define LCD_CLEARDISPLAY        0x01
#define LCD_RETURNHOME          0x02
#define LCD_ENTRYMODESET        0x04
#define LCD_DISPLAYCONTROL      0x08
#define LCD_CURSORSHIFT         0x10
#define LCD_FUNCTIONSET         0x20
#define LCD_SETCGRAMADDR        0x40
#define LCD_SETDDRAMADDR        0x80

/* Flags for display entry mode */
#define LCD_ENTRYRIGHT          0x00
#define LCD_ENTRYLEFT           0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

/* Flags for display on/off control */
#define LCD_DISPLAYON           0x04
#define LCD_DISPLAYOFF          0x00
#define LCD_CURSORON            0x02
#define LCD_CURSOROFF           0x00
#define LCD_BLINKON             0x01
#define LCD_BLINKOFF            0x00

/* Flags for display/cursor shift */
#define LCD_DISPLAYMOVE         0x08
#define LCD_CURSORMOVE          0x00
#define LCD_MOVERIGHT           0x04
#define LCD_MOVELEFT            0x00

/* Flags for function set */
#define LCD_8BITMODE            0x10
#define LCD_4BITMODE            0x00
#define LCD_2LINE               0x08
#define LCD_1LINE               0x00
#define LCD_5x10DOTS            0x04
#define LCD_5x8DOTS             0x00

/* Flags for backlight control */
#define LCD_BACKLIGHT           0x08
#define LCD_NOBACKLIGHT         0x00

#define En                      0x04  // Enable bit
#define Rw                      0x02  // Read/Write bit
#define Rs                      0x01  // Register select bit

/**
 * @brief Initialize the display system
 * @return 0 on success, negative error code on failure
 */
int display_init(void);

/**
 * @brief Update the display with current system status
 * @param state Pointer to the espresso state structure
 */
void display_update(struct espresso_state *state);

/**
 * @brief Send a command to the LCD
 * @param cmd Command byte to send
 */
void lcd_send_command(uint8_t cmd);

/**
 * @brief Send data to the LCD
 * @param data Data byte to send
 */
void lcd_send_data(uint8_t data);

/**
 * @brief Print a string to the LCD at the current cursor position
 * @param str String to print
 */
void lcd_print(const char *str);

/**
 * @brief Set cursor position on the LCD
 * @param col Column position (0-19)
 * @param row Row position (0-3)
 */
void lcd_set_cursor(uint8_t col, uint8_t row);

/**
 * @brief Clear the LCD display
 */
void lcd_clear(void);

#endif /* DISPLAY_H_ */