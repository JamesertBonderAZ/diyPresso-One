/*
 * Copyright (c) 2024 Espresso Controller Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <math.h>

#include "espresso_config.h"
#include "temperature.h"

LOG_MODULE_REGISTER(temperature, LOG_LEVEL_DBG);

static const struct device *spi_dev;
static const struct device *therm_rdy_dev;
static const struct device *therm_cs_dev;

static struct spi_config spi_cfg = {
	.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB |
		     SPI_MODE_CPOL | SPI_MODE_CPHA,
	.frequency = 1000000,  // 1 MHz
	.slave = 0,
};

static bool temp_initialized = false;

int temperature_init(void)
{
	spi_dev = DEVICE_DT_GET(DT_NODELABEL(spi1));
	therm_rdy_dev = DEVICE_DT_GET(DT_ALIAS(therm_rdy));
	therm_cs_dev = DEVICE_DT_GET(DT_ALIAS(therm_cs));

	if (!device_is_ready(spi_dev)) {
		LOG_ERR("SPI device not ready");
		return -ENODEV;
	}

	if (!device_is_ready(therm_rdy_dev) || !device_is_ready(therm_cs_dev)) {
		LOG_ERR("Thermocouple GPIO devices not ready");
		return -ENODEV;
	}

	/* Configure GPIO pins */
	gpio_pin_configure(therm_rdy_dev, PIN_THERM_RDY, GPIO_INPUT);
	gpio_pin_configure(therm_cs_dev, PIN_THERM_CS, GPIO_OUTPUT);

	// Initialize CS high (deselect)
	gpio_pin_set(therm_cs_dev, PIN_THERM_CS, 1);

	// Configure MAX31865
	uint8_t config_cmd[2] = {MAX31865_REG_CONFIG | 0x80,  // Write register
	                         MAX31865_CONFIG_BIAS | MAX31865_CONFIG_AUTO | MAX31865_CONFIG_3WIRE};

	struct spi_buf tx_buf = {.buf = config_cmd, .len = 2};
	struct spi_buf_set tx_bufs = {.buffers = &tx_buf, .count = 1};

	if (spi_write(spi_dev, &spi_cfg, &tx_bufs) != 0) {
		LOG_ERR("Failed to configure MAX31865");
		return -EIO;
	}

	temp_initialized = true;

	LOG_INF("Temperature sensor (MAX31865) initialized");
	return 0;
}

int max31865_spi_transfer(uint8_t *tx_buf, uint8_t *rx_buf, size_t len)
{
	if (!temp_initialized) {
		return -ENODEV;
	}

	gpio_pin_set(therm_cs_dev, PIN_THERM_CS, 0);

	struct spi_buf tx_spi_buf = {.buf = tx_buf, .len = len};
	struct spi_buf rx_spi_buf = {.buf = rx_buf, .len = len};
	struct spi_buf_set tx_bufs = {.buffers = &tx_spi_buf, .count = 1};
	struct spi_buf_set rx_bufs = {.buffers = &rx_spi_buf, .count = 1};

	int ret = spi_transceive(spi_dev, &spi_cfg, &tx_bufs, &rx_bufs);

	gpio_pin_set(therm_cs_dev, PIN_THERM_CS, 1);

	return ret;
}

float calculate_resistance(uint16_t rtd_val)
{
	// Convert RTD value to resistance
	// Formula: R = (RTD_val * Rref) / 32768
	return ((float)rtd_val * RREF) / 32768.0f;
}

float resistance_to_temperature(float resistance)
{
	// Simplified Callendar-Van Dusen equation for PT1000
	// Only valid for positive temperatures
	float z1, z2, z3, z4;
	float temp;

	z1 = -247.29140332f;
	z2 = 2.3474375659f;
	z3 = -7.2405256956E-04f;
	z4 = 8.5172928796E-07f;

	temp = z1 + (z2 * resistance) + (z3 * resistance * resistance) + (z4 * resistance * resistance * resistance);

	return temp;
}

float read_temperature(void)
{
	if (!temp_initialized) {
		return 20.0f;  // Return room temperature as default
	}

	uint8_t read_cmd[3] = {MAX31865_REG_RTD_MSB, 0x00, 0x00};
	uint8_t result[3] = {0};

	if (max31865_spi_transfer(read_cmd, result, 3) != 0) {
		LOG_WRN("Failed to read temperature");
		return 20.0f;
	}

	// Extract 15-bit RTD value (result[0] is MSB, result[1] is LSB)
	// Result[2] contains fault information in the lower bits
	uint16_t rtd_val = ((uint16_t)(result[0]) << 8) | result[1];
	rtd_val >>= 1;  // Remove fault bit

	float resistance = calculate_resistance(rtd_val);
	float temperature = resistance_to_temperature(resistance);

	return temperature;
}

void temperature_update(struct espresso_state *state)
{
	static uint32_t last_temp_read = 0;
	uint32_t current_time = k_uptime_get_32();

	if ((current_time - last_temp_read) < TEMP_READ_INTERVAL) {
		return;
	}

	last_temp_read = current_time;

	if (temp_initialized) {
		state->current_temperature = read_temperature();
	}
}

float calculate_pid(float setpoint, float current)
{
	float error = setpoint - current;
	
	// Proportional term
	float p_term = KP * error;
	
	// Integral term (with anti-windup)
	static float integral = 0.0f;
	integral += error * (TEMP_READ_INTERVAL / 1000.0f);  // Convert ms to seconds
	
	// Anti-windup: limit integral term
	if (integral > 100.0f/KI) {
		integral = 100.0f/KI;
	} else if (integral < -100.0f/KI) {
		integral = -100.0f/KI;
	}
	
	float i_term = KI * integral;
	
	// Derivative term
	static float last_error = 0.0f;
	float derivative = (error - last_error) / (TEMP_READ_INTERVAL / 1000.0f);
	float d_term = KD * derivative;
	
	// Store values for next iteration
	last_error = error;
	
	// Calculate total output
	float output = p_term + i_term + d_term;
	
	// Limit output to 0-100%
	if (output > 100.0f) {
		output = 100.0f;
	} else if (output < 0.0f) {
		output = 0.0f;
	}
	
	return output;
}

void temperature_control(struct espresso_state *state)
{
	if (!state->heating_enabled) {
		// Make sure heating is off if disabled
		gpio_pin_set(state->ssr_heater_dev, PIN_SSR_HEATER, 0);
		return;
	}

	// Simple on/off control with hysteresis
	if (state->current_temperature < (state->setpoint_temperature - TEMP_HYSTERESIS)) {
		// Turn on heating
		gpio_pin_set(state->ssr_heater_dev, PIN_SSR_HEATER, 1);
	} else if (state->current_temperature > (state->setpoint_temperature + TEMP_HYSTERESIS)) {
		// Turn off heating
		gpio_pin_set(state->ssr_heater_dev, PIN_SSR_HEATER, 0);
	}
	// Otherwise, maintain current state (within deadband)
}