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
#include "encoder.h"

LOG_MODULE_REGISTER(encoder, LOG_LEVEL_DBG);

static const struct device *enc_a_dev;
static const struct device *enc_b_dev;
static const struct device *enc_s_dev;

static int32_t encoder_counter = 0;
static uint8_t last_encoded = 0;
static int32_t last_counter = 0;
static uint32_t last_btn_press = 0;
static bool btn_pressed = false;

int encoder_init(void)
{
	enc_a_dev = DEVICE_DT_GET(DT_ALIAS(enc_a));
	enc_b_dev = DEVICE_DT_GET(DT_ALIAS(enc_b));
	enc_s_dev = DEVICE_DT_GET(DT_ALIAS(enc_s));

	if (!device_is_ready(enc_a_dev) ||
	    !device_is_ready(enc_b_dev) ||
	    !device_is_ready(enc_s_dev)) {
		LOG_ERR("One or more encoder devices not ready");
		return -ENODEV;
	}

	/* Configure GPIO pins */
	gpio_pin_configure(enc_a_dev, PIN_ENC_A, GPIO_INPUT | GPIO_PULL_UP);
	gpio_pin_configure(enc_b_dev, PIN_ENC_B, GPIO_INPUT | GPIO_PULL_UP);
	gpio_pin_configure(enc_s_dev, PIN_ENC_S, GPIO_INPUT | GPIO_PULL_UP);

	/* Set up interrupt handlers if needed */
	gpio_pin_interrupt_configure(enc_a_dev, PIN_ENC_A, GPIO_INT_EDGE_BOTH);
	gpio_pin_interrupt_configure(enc_b_dev, PIN_ENC_B, GPIO_INT_EDGE_BOTH);

	LOG_INF("Encoder initialized");
	return 0;
}

int get_encoder_state(void)
{
	uint8_t MSB = gpio_pin_get(enc_a_dev, PIN_ENC_A);  //MSB = most significant bit
	uint8_t LSB = gpio_pin_get(enc_b_dev, PIN_ENC_B);  //LSB = least significant bit

	uint8_t encoded = (MSB << 1) | LSB; //Convert the 2 pin value to a single number
	uint8_t sum = (last_encoded << 2) | encoded; //Add it to the previous encoded value

	if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
		encoder_counter++;
	} else if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
		encoder_counter--;
	}

	last_encoded = encoded;

	// Check for button press (active low)
	if (gpio_pin_get(enc_s_dev, PIN_ENC_S) == 0 && !btn_pressed) {
		btn_pressed = true;
		last_btn_press = k_uptime_get_32();
		return ENCODER_BTN_PRESS;
	} else if (gpio_pin_get(enc_s_dev, PIN_ENC_S) == 1) {
		btn_pressed = false;
	}

	// Check if encoder was rotated
	if (encoder_counter != last_counter) {
		if (encoder_counter > last_counter) {
			last_counter = encoder_counter;
			return ENCODER_CW;
		} else {
			last_counter = encoder_counter;
			return ENCODER_CCW;
		}
	}

	return ENCODER_NO_ACTION;
}

void handle_encoder_cw(struct espresso_state *state)
{
	// Adjust temperature setpoint upward
	state->setpoint_temperature += 0.5f;
	if (state->setpoint_temperature > 105.0f) {
		state->setpoint_temperature = 105.0f;
	}
	LOG_INF("Temperature setpoint increased to %.1f", state->setpoint_temperature);
}

void handle_encoder_ccw(struct espresso_state *state)
{
	// Adjust temperature setpoint downward
	state->setpoint_temperature -= 0.5f;
	if (state->setpoint_temperature < 80.0f) {
		state->setpoint_temperature = 80.0f;
	}
	LOG_INF("Temperature setpoint decreased to %.1f", state->setpoint_temperature);
}

void handle_encoder_btn(struct espresso_state *state)
{
	// Toggle heating
	toggle_heating(state);
	LOG_INF("Heating toggled");
}

void encoder_update(struct espresso_state *state)
{
	int enc_state = get_encoder_state();

	switch (enc_state) {
	case ENCODER_CW:
		handle_encoder_cw(state);
		break;
	case ENCODER_CCW:
		handle_encoder_ccw(state);
		break;
	case ENCODER_BTN_PRESS:
		handle_encoder_btn(state);
		break;
	default:
		// No action needed
		break;
	}
}