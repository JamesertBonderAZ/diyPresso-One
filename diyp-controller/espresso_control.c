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
#include "espresso_control.h"

LOG_MODULE_REGISTER(espresso_control, LOG_LEVEL_DBG);

int espresso_control_init(struct espresso_state *state)
{
	/* Get device pointers */
	state->brew_switch_dev = DEVICE_DT_GET(DT_ALIAS(brew_switch));
	state->ssr_pump_dev = DEVICE_DT_GET(DT_ALIAS(ssr_pump));
	state->ssr_heater_dev = DEVICE_DT_GET(DT_ALIAS(ssr_heater));

	if (!device_is_ready(state->brew_switch_dev) ||
	    !device_is_ready(state->ssr_pump_dev) ||
	    !device_is_ready(state->ssr_heater_dev)) {
		LOG_ERR("One or more devices not ready");
		return -ENODEV;
	}

	/* Configure GPIO pins */
	gpio_pin_configure(state->brew_switch_dev, PIN_BREW_SWITCH, GPIO_INPUT | GPIO_PULL_UP);
	gpio_pin_configure(state->ssr_pump_dev, PIN_SSR_PUMP, GPIO_OUTPUT);
	gpio_pin_configure(state->ssr_heater_dev, PIN_SSR_HEATER, GPIO_OUTPUT);

	/* Initialize state variables */
	state->brew_state = BREW_IDLE;
	state->brew_start_time = 0U;
	state->brew_duration = DEFAULT_BREW_TIME;
	state->target_temperature = DEFAULT_SETPOINT;
	state->target_weight = DEFAULT_TARGET_WEIGHT;
	state->heating_enabled = false;
	state->pump_enabled = false;
	
	state->current_temperature = 20.0f;  /* Initial room temperature */
	state->current_weight = 0.0f;
	state->setpoint_temperature = DEFAULT_SETPOINT;
	
	/* PID initialization */
	state->pid_error = 0.0f;
	state->pid_integral = 0.0f;
	state->pid_derivative = 0.0f;
	state->pid_last_error = 0.0f;
	state->pid_output = 0.0f;
	
	/* UI initialization */
	state->encoder_value = 0;
	state->last_encoder_value = 0;
	state->encoder_button_pressed = false;

	LOG_INF("Espresso control initialized");
	return 0;
}

void start_brewing(struct espresso_state *state)
{
	if (state->brew_state == BREW_IDLE) {
		state->brew_state = BREW_PRE_INFUSION;
		state->brew_start_time = k_uptime_get_32();
		state->pump_enabled = true;
		
		/* Turn on pump */
		gpio_pin_set(state->ssr_pump_dev, PIN_SSR_PUMP, 1);
		
		LOG_INF("Brewing started");
	}
}

void stop_brewing(struct espresso_state *state)
{
	state->brew_state = BREW_IDLE;
	state->pump_enabled = false;
	
	/* Turn off pump */
	gpio_pin_set(state->ssr_pump_dev, PIN_SSR_PUMP, 0);
	
	LOG_INF("Brewing stopped");
}

void toggle_heating(struct espresso_state *state)
{
	state->heating_enabled = !state->heating_enabled;
	gpio_pin_set(state->ssr_heater_dev, PIN_SSR_HEATER, state->heating_enabled ? 1 : 0);
	
	LOG_INF("Heating %s", state->heating_enabled ? "enabled" : "disabled");
}

void espresso_control_update(struct espresso_state *state)
{
	uint32_t current_time = k_uptime_get_32();
	
	/* Check for brew switch activation */
	if (gpio_pin_get(state->brew_switch_dev, PIN_BREW_SWITCH) == 0) {
		/* Switch pressed (active low) */
		if (state->brew_state == BREW_IDLE) {
			start_brewing(state);
		} else if (state->brew_state != BREW_IDLE) {
			stop_brewing(state);
		}
	}
	
	/* Update brewing state machine */
	switch (state->brew_state) {
	case BREW_IDLE:
		/* Already handled above */
		break;
		
	case BREW_PRE_INFUSION:
		/* Pre-infusion phase: short pulse to expand grounds */
		if ((current_time - state->brew_start_time) >= 5000) {  /* 5 seconds */
			state->brew_state = BREW_MAIN;
		}
		break;
		
	case BREW_MAIN:
		/* Main brewing phase */
		if ((current_time - state->brew_start_time) >= state->brew_duration) {
			state->brew_state = BREW_FINISHING;
			gpio_pin_set(state->ssr_pump_dev, PIN_SSR_PUMP, 0);
			state->pump_enabled = false;
		}
		break;
		
	case BREW_FINISHING:
		/* Brief pause after brewing */
		if ((current_time - state->brew_start_time) >= state->brew_duration + 2000) {
			state->brew_state = BREW_IDLE;
		}
		break;
		
	case BREW_FLUSHING:
		/* Flushing mode for cleaning */
		if ((current_time - state->brew_start_time) >= 10000) {  /* 10 seconds */
			state->brew_state = BREW_IDLE;
			gpio_pin_set(state->ssr_pump_dev, PIN_SSR_PUMP, 0);
			state->pump_enabled = false;
		}
		break;
		
	default:
		state->brew_state = BREW_IDLE;
		break;
	}
}