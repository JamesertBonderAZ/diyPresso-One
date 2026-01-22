/*
 * Copyright (c) 2024 Espresso Controller Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "espresso_config.h"
#include "scale.h"

LOG_MODULE_REGISTER(scale, LOG_LEVEL_DBG);

static const struct device *hx711_clk_dev;
static const struct device *hx711_dat_dev;

static float calibration_factor = 1000.0f;  // Calibration factor - needs adjustment
static float offset = 0.0f;                 // Zero offset
static bool is_initialized = false;

int scale_init(void)
{
	hx711_clk_dev = DEVICE_DT_GET(DT_ALIAS(hx711_clk));
	hx711_dat_dev = DEVICE_DT_GET(DT_ALIAS(hx711_dat));

	if (!device_is_ready(hx711_clk_dev) || !device_is_ready(hx711_dat_dev)) {
		LOG_ERR("HX711 devices not ready");
		return -ENODEV;
	}

	/* Configure GPIO pins */
	gpio_pin_configure(hx711_clk_dev, PIN_HX711_CLK, GPIO_OUTPUT);
	gpio_pin_configure(hx711_dat_dev, PIN_HX711_DAT, GPIO_INPUT);

	// Initialize HX711 - set clock low initially
	gpio_pin_set(hx711_clk_dev, PIN_HX711_CLK, 0);

	// Allow some time for HX711 to initialize
	k_msleep(100);

	is_initialized = true;

	LOG_INF("Scale/HX711 initialized");
	return 0;
}

int32_t hx711_read_raw(void)
{
	if (!is_initialized) {
		return 0;
	}

	// Wait for HX711 to be ready (data pin goes low)
	uint32_t timeout = 0;
	while (gpio_pin_get(hx711_dat_dev, PIN_HX711_DAT) && timeout < 1000) {
		k_usleep(10);
		timeout++;
	}

	if (timeout >= 1000) {
		LOG_WRN("HX711 read timeout");
		return 0;
	}

	int32_t data = 0;

	// Read 24 bits of data
	for (int i = 0; i < 24; i++) {
		gpio_pin_set(hx711_clk_dev, PIN_HX711_CLK, 1);
		k_usleep(1);
		data = data << 1;
		if (gpio_pin_get(hx711_dat_dev, PIN_HX711_DAT)) {
			data |= 1;
		}
		gpio_pin_set(hx711_clk_dev, PIN_HX711_CLK, 0);
		k_usleep(1);
	}

	// Set gain to 128 (send 25th pulse)
	gpio_pin_set(hx711_clk_dev, PIN_HX711_CLK, 1);
	k_usleep(1);
	gpio_pin_set(hx711_clk_dev, PIN_HX711_CLK, 0);

	// Convert to signed 24-bit value
	if (data & 0x800000) {
		data |= 0xFF000000;  // Sign extend to 32-bit
	}

	return data;
}

float raw_to_weight(int32_t raw_value)
{
	return (raw_value - offset) / calibration_factor;
}

void scale_tare(void)
{
	int32_t raw_value = hx711_read_raw();
	offset = raw_value;
	LOG_INF("Scale tared, offset set to %d", (int)offset);
}

void scale_update(struct espresso_state *state)
{
	static uint32_t last_scale_read = 0;
	uint32_t current_time = k_uptime_get_32();

	if ((current_time - last_scale_read) < SCALE_READ_INTERVAL) {
		return;
	}

	last_scale_read = current_time;

	int32_t raw_value = hx711_read_raw();
	if (raw_value != 0) {
		state->current_weight = raw_to_weight(raw_value);
	}
}