/*
 * PID controller for diyPresso
 * Ported to Zephyr RTOS
 */

#include <zephyr/kernel.h>
#include "pid.h"

int pid_init(struct dp_pid *pid)
{
    if (!pid) {
        return -EINVAL;
    }

    pid->kp = 0.0;
    pid->ki = 0.0;
    pid->kd = 0.0;
    pid->integral = 0.0;
    pid->last_error = 0.0;
    pid->output_min = -100.0;
    pid->output_max = 100.0;
    pid->windup_min = -100.0;
    pid->windup_max = 100.0;

    return 0;
}

void pid_set_coefficients(struct dp_pid *pid, double kp, double ki, double kd)
{
    if (pid) {
        pid->kp = kp;
        pid->ki = ki;
        pid->kd = kd;
    }
}

void pid_set_limits(struct dp_pid *pid, double min_out, double max_out)
{
    if (pid) {
        pid->output_min = min_out;
        pid->output_max = max_out;
    }
}

void pid_set_windup_limits(struct dp_pid *pid, double min_windup, double max_windup)
{
    if (pid) {
        pid->windup_min = min_windup;
        pid->windup_max = max_windup;
    }
}

double pid_compute(struct dp_pid *pid, double setpoint, double measured_value, uint32_t dt_ms)
{
    if (!pid || dt_ms == 0) {
        return 0.0;
    }

    double error = setpoint - measured_value;
    double dt_sec = (double)dt_ms / 1000.0;

    // Proportional term
    double proportional = pid->kp * error;

    // Integral term
    pid->integral += error * dt_sec;
    
    // Apply windup protection
    if (pid->integral > pid->windup_max) {
        pid->integral = pid->windup_max;
    } else if (pid->integral < pid->windup_min) {
        pid->integral = pid->windup_min;
    }

    double integral = pid->ki * pid->integral;

    // Derivative term
    double derivative = pid->kd * (error - pid->last_error) / dt_sec;
    pid->last_error = error;

    // Compute output
    double output = proportional + integral + derivative;

    // Apply output limits
    if (output > pid->output_max) {
        output = pid->output_max;
    } else if (output < pid->output_min) {
        output = pid->output_min;
    }

    return output;
}