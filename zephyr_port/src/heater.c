/*
 * Heater control for diyPresso
 * Ported to Zephyr RTOS
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/printk.h>

#include "heater.h"
#include "gpio_config.h"

static struct heater_state heater_state;

/* Timer callback for heater control */
static void heater_control_timer(struct k_timer *timer)
{
    heater_control();
}

int heater_init(void)
{
    // Initialize heater state
    heater_state.power = 0.0;
    heater_state.average_power = 0.0;
    heater_state.pwm_period_us = HEATER_PWM_PERIOD_DEFAULT_US;
    heater_state.time_on = 0;
    heater_state.is_on = false;
    
    // Get PWM device from devicetree
    // Note: We'll use GPIO bit-banging for SSR control since STM32L031K6 might not have enough PWM channels
    const struct device *ssr_dev = DEVICE_DT_GET(DT_ALIAS(ssr_heater_gpio_port));
    uint32_t ssr_pin = DT_GPIO_PIN(DT_ALIAS(ssr_heater_gpio_port), gpios);
    
    if (!device_is_ready(ssr_dev)) {
        return -ENODEV;
    }
    
    int ret = gpio_pin_configure(ssr_dev, ssr_pin, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        return ret;
    }
    
    // Set initial state to OFF
    gpio_pin_set(ssr_dev, ssr_pin, 0);
    
    // Initialize control timer
    k_timer_init(&heater_state.control_timer, heater_control_timer, NULL);
    k_timer_start(&heater_state.control_timer, K_MSEC(100), K_MSEC(100)); // 100ms period
    
    return 0;
}

void heater_control(void)
{
    const struct device *ssr_dev = DEVICE_DT_GET(DT_ALIAS(ssr_heater_gpio_port));
    uint32_t ssr_pin = DT_GPIO_PIN(DT_ALIAS(ssr_heater_gpio_port), gpios);
    
    if (!device_is_ready(ssr_dev)) {
        return;
    }
    
    // Simple software PWM implementation
    static uint32_t pwm_counter = 0;
    static uint32_t last_toggle_time = 0;
    
    uint32_t current_time = k_uptime_get_32();
    uint32_t elapsed = current_time - last_toggle_time;
    
    // Calculate duty cycle (power level)
    double duty_cycle = heater_state.power / 100.0;
    uint32_t on_time = (uint32_t)(heater_state.pwm_period_us * duty_cycle / 1000) * 1000; // Convert to ms
    
    if (elapsed >= heater_state.pwm_period_us / 1000) { // Period has passed
        pwm_counter = 0;
        last_toggle_time = current_time;
        
        if (heater_state.power > 0.0 && heater_state.power < 100.0) {
            // Toggle based on duty cycle
            if (pwm_counter < on_time) {
                gpio_pin_set(ssr_dev, ssr_pin, 1);
                heater_state.is_on = true;
            } else {
                gpio_pin_set(ssr_dev, ssr_pin, 0);
                heater_state.is_on = false;
            }
        } else if (heater_state.power >= 100.0) {
            // Full power
            gpio_pin_set(ssr_dev, ssr_pin, 1);
            heater_state.is_on = true;
        } else {
            // Zero power
            gpio_pin_set(ssr_dev, ssr_pin, 0);
            heater_state.is_on = false;
        }
    } else {
        // Continue current state
        if (heater_state.power > 0.0 && heater_state.power < 100.0) {
            if (pwm_counter < on_time) {
                gpio_pin_set(ssr_dev, ssr_pin, 1);
                heater_state.is_on = true;
            } else {
                gpio_pin_set(ssr_dev, ssr_pin, 0);
                heater_state.is_on = false;
            }
        }
        pwm_counter += elapsed;
    }
}

void heater_pwm_period(double seconds)
{
    uint32_t period_us = (uint32_t)(seconds * 1000000);
    heater_state.pwm_period_us = period_us;
    
    // Clamp to valid range
    if (heater_state.pwm_period_us < HEATER_PWM_PERIOD_MIN_US) {
        heater_state.pwm_period_us = HEATER_PWM_PERIOD_MIN_US;
    } else if (heater_state.pwm_period_us > HEATER_PWM_PERIOD_MAX_US) {
        heater_state.pwm_period_us = HEATER_PWM_PERIOD_MAX_US;
    }
}

void heater_on(void)
{
    heater_set_power(100.0);
}

void heater_off(void)
{
    heater_set_power(0.0);
}

void heater_set_power(double power_percent)
{
    if (power_percent < 0.0) {
        power_percent = 0.0;
    } else if (power_percent > 100.0) {
        power_percent = 100.0;
    }
    
    heater_state.power = power_percent;
}

double heater_get_power(void)
{
    return heater_state.power;
}

double heater_get_average_power(void)
{
    return heater_state.average_power;
}

bool heater_is_on(void)
{
    return heater_state.is_on;
}

double heater_get_pwm_period(void)
{
    return (double)heater_state.pwm_period_us / 1000000.0;
}