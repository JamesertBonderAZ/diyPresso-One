/*
 * Copyright (c) 2024 Espresso Controller Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <string.h>

#include "espresso_config.h"
#include "display.h"

LOG_MODULE_REGISTER(display, LOG_LEVEL_DBG);

static const struct device *i2c_dev;
static uint32_t last_display_update = 0;

int display_init(void)
{
	i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));
	
	if (!device_is_ready(i2c_dev)) {
		LOG_ERR("I2C device not ready");
		return -ENODEV;
	}
	
	// Initialize LCD
	k_msleep(50);  // Wait for LCD to power up
	
	// Send initialization sequence
	lcd_send_command(0x33);  // Initialize
	k_msleep(5);
	lcd_send_command(0x32);  // 4-bit mode
	k_msleep(5);
	lcd_send_command(0x28);  // 2 lines, 5x8 dots
	k_msleep(5);
	lcd_send_command(0x0C);  // Display on, cursor off, blink off
	k_msleep(5);
	lcd_send_command(0x06);  // Increment cursor
	k_msleep(5);
	lcd_clear();             // Clear display
	
	LOG_INF("Display initialized");
	return 0;
}

static void expander_write(uint8_t data)
{
	uint8_t buffer[1] = {data | LCD_BACKLIGHT};
	
	if (i2c_write(i2c_dev, buffer, 1, LCD_ADDR) != 0) {
		LOG_WRN("Failed to write to LCD");
	}
}

static void pulse_enable(uint8_t data)
{
	expander_write(data | En);
	k_usleep(1);
	expander_write(data & ~En);
	k_usleep(50);
}

static void write_4_bits(uint8_t value)
{
	expander_write(value);
	pulse_enable(value);
}

void lcd_send_command(uint8_t cmd)
{
	uint8_t high_nib = cmd & 0xF0;
	uint8_t low_nib = (cmd << 4) & 0xF0;
	
	write_4_bits(high_nib);
	write_4_bits(low_nib);
}

void lcd_send_data(uint8_t data)
{
	uint8_t high_nib = data & 0xF0;
	uint8_t low_nib = (data << 4) & 0xF0;
	
	write_4_bits(high_nib | Rs);
	write_4_bits(low_nib | Rs);
}

void lcd_clear(void)
{
	lcd_send_command(LCD_CLEARDISPLAY);
	k_msleep(2);
}

void lcd_set_cursor(uint8_t col, uint8_t row)
{
	uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
	uint8_t pos = 0x80 | (col + row_offsets[row]);
	lcd_send_command(pos);
}

void lcd_print(const char *str)
{
	for (int i = 0; str[i] != '\0'; i++) {
		lcd_send_data((uint8_t)str[i]);
	}
}

void display_update(struct espresso_state *state)
{
	uint32_t current_time = k_uptime_get_32();
	
	// Update display at defined interval
	if ((current_time - last_display_update) < DISPLAY_UPDATE_INTERVAL) {
		return;
	}
	
	last_display_update = current_time;
	
	// Clear display
	lcd_clear();
	
	// Line 1: Temperature info
	char temp_str[21];
	snprintf(temp_str, sizeof(temp_str), "Temp: %.1f/%.1f C", 
	         state->current_temperature, state->setpoint_temperature);
	lcd_set_cursor(0, 0);
	lcd_print(temp_str);
	
	// Line 2: Weight info
	char weight_str[21];
	snprintf(weight_str, sizeof(weight_str), "Weight: %.1fg", state->current_weight);
	lcd_set_cursor(0, 1);
	lcd_print(weight_str);
	
	// Line 3: Brewing status
	lcd_set_cursor(0, 2);
	switch (state->brew_state) {
	case BREW_IDLE:
		lcd_print("Status: IDLE        ");
		break;
	case BREW_PRE_INFUSION:
		lcd_print("Status: PRE-INFUSION");
		break;
	case BREW_MAIN:
		lcd_print("Status: BREWING     ");
		break;
	case BREW_FINISHING:
		lcd_print("Status: FINISHING   ");
		break;
	case BREW_FLUSHING:
		lcd_print("Status: FLUSHING    ");
		break;
	default:
		lcd_print("Status: UNKNOWN     ");
		break;
	}
	
	// Line 4: Settings/controls
	char settings_str[21];
	snprintf(settings_str, sizeof(settings_str), "Set:%.0fC Wgt:%.0fg", 
	         state->setpoint_temperature, state->target_weight);
	lcd_set_cursor(0, 3);
	lcd_print(settings_str);
}