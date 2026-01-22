/*
 * Heater control for diyPresso
 * Ported to Zephyr RTOS
 */

#ifndef HEATER_H_
#define HEATER_H_

#include <zephyr/kernel.h>

/* Heater configuration */
#define HEATER_PWM_PERIOD_DEFAULT_US 1000000  /* 1 second default PWM period */
#define HEATER_PWM_PERIOD_MIN_US 100000       /* 0.1 second minimum */
#define HEATER_PWM_PERIOD_MAX_US 10000000     /* 10 second maximum */

/* Heater state structure */
struct heater_state {
    double power;           /* Power level [0..100%] */
    double average_power;   /* Average power over time */
    uint32_t pwm_period_us; /* PWM period in microseconds */
    uint32_t time_on;       /* Time in current state (microseconds) */
    bool is_on;             /* Current state of heater */
    struct k_timer control_timer;
    const struct device *pwm_dev;
    uint32_t pwm_channel;
};

/* Public API */
int heater_init(void);
void heater_control(void);
void heater_pwm_period(double seconds);
void heater_on(void);
void heater_off(void);
void heater_set_power(double power_percent);
double heater_get_power(void);
double heater_get_average_power(void);
bool heater_is_on(void);
double heater_get_pwm_period(void);

#endif /* HEATER_H_ */