/*
 * Copyright (c) 2024 Espresso Controller Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SCALE_H_
#define SCALE_H_

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include "espresso_control.h"

/* HX711 constants */
#define HX711_GAIN_CHANNEL_A_128  0x01  // Channel A with gain factor 128
#define HX711_GAIN_CHANNEL_B_32   0x02  // Channel B with gain factor 32
#define HX711_GAIN_CHANNEL_A_64   0x03  // Channel A with gain factor 64

/**
 * @brief Initialize the scale system (HX711)
 * @return 0 on success, negative error code on failure
 */
int scale_init(void);

/**
 * @brief Update scale reading
 * @param state Pointer to the espresso state structure
 */
void scale_update(struct espresso_state *state);

/**
 * @brief Read raw value from HX711
 * @return Raw 24-bit value from HX711
 */
int32_t hx711_read_raw(void);

/**
 * @brief Convert raw value to weight in grams
 * @param raw_value Raw value from HX711
 * @return Weight in grams
 */
float raw_to_weight(int32_t raw_value);

/**
 * @brief Tare the scale
 */
void scale_tare(void);

#endif /* SCALE_H_ */