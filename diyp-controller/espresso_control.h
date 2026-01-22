/*
 * Copyright (c) 2024 Espresso Controller Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ESPRESSO_CONTROL_H_
#define ESPRESSO_CONTROL_H_

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

/* Brewing states */
enum brewing_state {
	BREW_IDLE = 0,
	BREW_PRE_INFUSION,
	BREW_MAIN,
	BREW_FINISHING,
	BREW_FLUSHING,
	BREW_ERROR
};

/* Espresso controller state structure */
struct espresso_state {
	/* Hardware devices */
	const struct device *brew_switch_dev;
	const struct device *ssr_pump_dev;
	const struct device *ssr_heater_dev;
	
	/* State variables */
	enum brewing_state brew_state;
	uint32_t brew_start_time;
	uint32_t brew_duration;
	float target_temperature;
	float target_weight;
	bool heating_enabled;
	bool pump_enabled;
	
	/* Process variables */
	float current_temperature;
	float current_weight;
	float setpoint_temperature;
	
	/* PID control variables */
	float pid_error;
	float pid_integral;
	float pid_derivative;
	float pid_last_error;
	float pid_output;
	
	/* User interface */
	int32_t encoder_value;
	int32_t last_encoder_value;
	bool encoder_button_pressed;
};

/**
 * @brief Initialize the espresso control system
 * @param state Pointer to the espresso state structure
 * @return 0 on success, negative error code on failure
 */
int espresso_control_init(struct espresso_state *state);

/**
 * @brief Update the espresso control logic
 * @param state Pointer to the espresso state structure
 */
void espresso_control_update(struct espresso_state *state);

/**
 * @brief Start brewing process
 * @param state Pointer to the espresso state structure
 */
void start_brewing(struct espresso_state *state);

/**
 * @brief Stop brewing process
 * @param state Pointer to the espresso state structure
 */
void stop_brewing(struct espresso_state *state);

/**
 * @brief Toggle heating element
 * @param state Pointer to the espresso state structure
 */
void toggle_heating(struct espresso_state *state);

#endif /* ESPRESSO_CONTROL_H_ */