/*
 * Copyright (c) 2024 Espresso Controller Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TEMPERATURE_H_
#define TEMPERATURE_H_

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>

#include "espresso_control.h"

/* MAX31865 registers */
#define MAX31865_REG_RTD_MSB    0x01
#define MAX31865_REG_RTD_LSB    0x02
#define MAX31865_REG_FAULT_MSB  0x07
#define MAX31865_REG_FAULT_LSB  0x08
#define MAX31865_REG_CONFIG     0x00

/* MAX31865 configuration bits */
#define MAX31865_CONFIG_BIAS    0x80
#define MAX31865_CONFIG_AUTO    0x40
#define MAX31865_CONFIG_ONE_SHOT 0x20
#define MAX31865_CONFIG_3WIRE   0x10
#define MAX31865_CONFIG_FAULT   0x02
#define MAX31865_CONFIG_FAULT_CLEAR 0x80

/* Resistance to temperature conversion constants for PT1000 */
#define RREF                    430.0f   // Reference resistor value in ohms
#define RNOMINAL                1000.0f  // PT1000 nominal resistance at 0°C

/**
 * @brief Initialize the temperature measurement system (MAX31865)
 * @return 0 on success, negative error code on failure
 */
int temperature_init(void);

/**
 * @brief Update temperature reading
 * @param state Pointer to the espresso state structure
 */
void temperature_update(struct espresso_state *state);

/**
 * @brief Read temperature from PT1000 via MAX31865
 * @return Temperature in degrees Celsius
 */
float read_temperature(void);

/**
 * @brief Control heating element based on PID algorithm
 * @param state Pointer to the espresso state structure
 */
void temperature_control(struct espresso_state *state);

/**
 * @brief Calculate PID output
 * @param setpoint Desired temperature
 * @param current Current temperature
 * @return PID output value (0-100%)
 */
float calculate_pid(float setpoint, float current);

/**
 * @brief SPI transfer function for MAX31865 communication
 * @param tx_buf Transmit buffer
 * @param rx_buf Receive buffer
 * @param len Buffer length
 * @return 0 on success, negative error code on failure
 */
int max31865_spi_transfer(uint8_t *tx_buf, uint8_t *rx_buf, size_t len);

#endif /* TEMPERATURE_H_ */