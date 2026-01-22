/*
 * PID controller for diyPresso
 * Ported to Zephyr RTOS
 */

#ifndef PID_H_
#define PID_H_

#include <zephyr/kernel.h>

/* PID state structure */
struct dp_pid {
    double kp;           /* Proportional gain */
    double ki;           /* Integral gain */
    double kd;           /* Derivative gain */
    double integral;     /* Integral term */
    double last_error;   /* Previous error value */
    double output_min;   /* Minimum output value */
    double output_max;   /* Maximum output value */
    double windup_min;   /* Minimum windup limit */
    double windup_max;   /* Maximum windup limit */
};

/* Public API */
int pid_init(struct dp_pid *pid);
void pid_set_coefficients(struct dp_pid *pid, double kp, double ki, double kd);
void pid_set_limits(struct dp_pid *pid, double min_out, double max_out);
void pid_set_windup_limits(struct dp_pid *pid, double min_windup, double max_windup);
double pid_compute(struct dp_pid *pid, double setpoint, double measured_value, uint32_t dt_ms);

#endif /* PID_H_ */