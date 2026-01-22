/*
 * Boiler control for diyPresso
 * Ported to Zephyr RTOS
 */

#ifndef BOILER_H_
#define BOILER_H_

#include <zephyr/kernel.h>
#include "pid.h"

/* Temperature limits (in Celsius) */
#define BOILER_TEMP_WINDOW      10.0    /* In temperature range */
#define BOILER_TEMP_LIMIT_HIGH  108.0   /* High temperature limit */
#define BOILER_TEMP_LIMIT_LOW   1.0     /* Low temperature limit */
#define BOILER_TEMP_MIN_BREW    10.0    /* Minimum brew temperature */

/* Timeouts (in milliseconds) */
#define BOILER_TIMEOUT_HEATING      (600 * 1000)    /* 10 minutes max heating */
#define BOILER_TIMEOUT_BREW         (3 * 60 * 1000) /* 3 minutes max brew */
#define BOILER_TIMEOUT_READY        (2 * 60 * 60 * 1000) /* 2 hours max ready state */
#define BOILER_TIMEOUT_CONTROL_MSEC (10 * 1000)     /* 10 seconds max between controls */

/* PID Windup limits */
#define BOILER_WINDUP_LIMIT_MIN -7.0  /* Windup limits in % */
#define BOILER_WINDUP_LIMIT_MAX 7.0

/* Boiler error types */
typedef enum {
    BOILER_ERROR_NONE,
    BOILER_ERROR_RTD,
    BOILER_ERROR_TIMEOUT_BREW,
    BOILER_ERROR_TIMEOUT_HEATING,
    BOILER_ERROR_READY_TIMEOUT,
    BOILER_ERROR_SSR_TIMEOUT,
    BOILER_ERROR_OVER_TEMP,
    BOILER_ERROR_UNDER_TEMP,
    BOILER_ERROR_CONTROL_TIMEOUT,
    BOILER_ERROR_UNKNOWN,
} boiler_error_t;

/* Boiler state machine states */
typedef enum {
    BOILER_STATE_OFF,
    BOILER_STATE_HEATING,
    BOILER_STATE_READY,
    BOILER_STATE_BREW,
    BOILER_STATE_ERROR,
} boiler_state_t;

/* Boiler state structure */
struct boiler_state {
    struct dp_pid pid;
    double act_temp;              /* Actual temperature */
    double set_temp;              /* Target temperature */
    double act_power;             /* Actual power output */
    double ff_heat;              /* Feedforward during heating */
    double ff_ready;             /* Feedforward during ready */
    double ff_brew;              /* Feedforward during brew */
    bool is_on;                  /* Whether boiler is active */
    bool is_brewing;             /* Whether in brewing mode */
    unsigned long last_control_time; /* Last control update time */
    boiler_error_t error;        /* Current error state */
    int rtd_error;               /* RTD sensor errors */
    boiler_state_t current_state; /* Current state machine state */
    unsigned long state_entry_time; /* Time when entered current state */
};

/* Public API */
int boiler_controller_init(void);
void boiler_control(void);
void boiler_on(void);
void boiler_off(void);
void boiler_start_brew(void);
void boiler_stop_brew(void);
bool boiler_is_on(void);
bool boiler_is_ready(void);
bool boiler_is_error(void);
bool boiler_is_brewing(void);
void boiler_clear_error(void);
double boiler_get_set_temp(void);
double boiler_set_temp(double temp);
double boiler_get_act_temp(void);
double boiler_get_act_power(void);
double boiler_get_ff_heat(void);
double boiler_set_ff_heat(double ff);
double boiler_get_ff_ready(void);
double boiler_set_ff_ready(double ff);
double boiler_get_ff_brew(void);
double boiler_set_ff_brew(double ff);
void boiler_set_pid(double p, double i, double d);
const char* boiler_get_error_text(void);
const char* boiler_get_state_name(void);
boiler_error_t boiler_get_error(void);

#endif /* BOILER_H_ */