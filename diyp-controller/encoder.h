/*
 * Copyright (c) 2024 Espresso Controller Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ENCODER_H_
#define ENCODER_H_

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include "espresso_control.h"

/* Encoder state values */
#define ENCODER_NO_ACTION   0
#define ENCODER_CW          1
#define ENCODER_CCW         2
#define ENCODER_BTN_PRESS   3

/**
 * @brief Initialize the rotary encoder
 * @return 0 on success, negative error code on failure
 */
int encoder_init(void);

/**
 * @brief Update encoder state and handle events
 * @param state Pointer to the espresso state structure
 */
void encoder_update(struct espresso_state *state);

/**
 * @brief Get current encoder state
 * @return Encoder state (CW, CCW, button press, or no action)
 */
int get_encoder_state(void);

/**
 * @brief Handle encoder clockwise rotation
 * @param state Pointer to the espresso state structure
 */
void handle_encoder_cw(struct espresso_state *state);

/**
 * @brief Handle encoder counter-clockwise rotation
 * @param state Pointer to the espresso state structure
 */
void handle_encoder_ccw(struct espresso_state *state);

/**
 * @brief Handle encoder button press
 * @param state Pointer to the espresso state structure
 */
void handle_encoder_btn(struct espresso_state *state);

#endif /* ENCODER_H_ */