/*
 * Copyright (c) 2024 Espresso Controller Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>

#include "espresso_config.h"
#include "espresso_control.h"
#include "display.h"
#include "encoder.h"
#include "scale.h"
#include "temperature.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* Global state for the espresso controller */
struct espresso_state es_state;

int main(void)
{
	printk("Starting Espresso Controller Application\n");

	/* Initialize subsystems */
	if (espresso_control_init(&es_state) != 0) {
		LOG_ERR("Failed to initialize espresso control");
		return -1;
	}

	if (display_init() != 0) {
		LOG_ERR("Failed to initialize display");
		return -1;
	}

	if (encoder_init() != 0) {
		LOG_ERR("Failed to initialize encoder");
		return -1;
	}

	if (scale_init() != 0) {
		LOG_ERR("Failed to initialize scale");
		return -1;
	}

	if (temperature_init() != 0) {
		LOG_ERR("Failed to initialize temperature system");
		return -1;
	}

	/* Main application loop */
	while (1) {
		/* Update encoder state */
		encoder_update(&es_state);

		/* Update temperature reading */
		temperature_update(&es_state);

		/* Update scale reading */
		scale_update(&es_state);

		/* Update brewing logic */
		espresso_control_update(&es_state);

		/* Update display */
		display_update(&es_state);

		/* Control heating element */
		temperature_control(&es_state);

		k_msleep(MAIN_LOOP_INTERVAL);
	}

	return 0;
}