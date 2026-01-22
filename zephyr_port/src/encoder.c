/*
 * Rotary encoder driver for diyPresso
 * Ported to Zephyr RTOS
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "encoder.h"
#include "gpio_config.h"

static struct encoder_state enc_state;

/* Timer callback for encoder processing */
static void encoder_timer_callback(struct k_timer *timer)
{
    // Get current encoder state
    int enc_a = GPIO_GET(enc_state.port_a, enc_state.pin_a);
    int enc_b = GPIO_GET(enc_state.port_b, enc_state.pin_b);
    
    if (enc_a < 0 || enc_b < 0) {
        return; // Error reading pins
    }
    
    // Button debouncing and handling
    enc_state.enc_switch = (enc_state.enc_switch << 1) | (GPIO_GET(enc_state.port_btn, enc_state.pin_btn) ? 0 : 1);
    enc_state.button_state = (enc_state.enc_switch & BUTTON_DEGLITCH_BITS) == BUTTON_DEGLITCH_BITS ? 1 : 0;
    
    if (enc_state.button_state && !enc_state.enc_prev_button) { // falling edge
        enc_state.button_count++;
    }
    enc_state.enc_prev_button = enc_state.button_state;
    
    if (enc_state.button_state) {
        enc_state.button_time++;
    } else {
        enc_state.button_time = 0;
    }
    
    // Encoder debouncing and handling
    uint32_t val = 0;
    val |= enc_a ? 1 : 0;
    val |= enc_b ? 2 : 0;
    
    if (val & 1) {
        enc_state.afilt++;
    } else {
        enc_state.afilt--;
    }
    if (enc_state.afilt > ENCODER_DEGLITCH_COUNT) { 
        enc_state.afilt = ENCODER_DEGLITCH_COUNT; 
        enc_state.prev_enc_state |= 1; 
    }
    if (enc_state.afilt < -ENCODER_DEGLITCH_COUNT) { 
        enc_state.afilt = -ENCODER_DEGLITCH_COUNT; 
        enc_state.prev_enc_state &= 2; 
    }
    
    if (val & 2) {
        enc_state.bfilt++;
    } else {
        enc_state.bfilt--;
    }
    if (enc_state.bfilt > ENCODER_DEGLITCH_COUNT) { 
        enc_state.bfilt = ENCODER_DEGLITCH_COUNT; 
        enc_state.prev_enc_state |= 2; 
    }
    if (enc_state.bfilt < -ENCODER_DEGLITCH_COUNT) { 
        enc_state.bfilt = -ENCODER_DEGLITCH_COUNT; 
        enc_state.prev_enc_state &= 1; 
    }
    
    uint32_t cur = enc_state.prev_enc_state;
    if (((enc_state.prev_enc_state & 1) == 1) && ((cur & 1) == 0)) {
        enc_state.position += (cur & 2 ? -1 : 1);
    }
    enc_state.prev_enc_state = cur;
    
    enc_state.timer_count++;
}

int encoder_init(void)
{
    // Get device pointers from devicetree
    enc_state.port_a = DEVICE_DT_GET(DT_ALIAS(enc_a_gpio_port));
    enc_state.port_b = DEVICE_DT_GET(DT_ALIAS(enc_b_gpio_port));
    enc_state.port_btn = DEVICE_DT_GET(DT_ALIAS(enc_btn_gpio_port));
    
    if (!device_is_ready(enc_state.port_a) ||
        !device_is_ready(enc_state.port_b) ||
        !device_is_ready(enc_state.port_btn)) {
        return -ENODEV;
    }
    
    // Get pin numbers from devicetree
    enc_state.pin_a = DT_GPIO_PIN(DT_ALIAS(enc_a_gpio_port), gpios);
    enc_state.pin_b = DT_GPIO_PIN(DT_ALIAS(enc_b_gpio_port), gpios);
    enc_state.pin_btn = DT_GPIO_PIN(DT_ALIAS(enc_btn_gpio_port), gpios);
    
    // Configure GPIO pins
    int ret = gpio_pin_configure(enc_state.port_a, enc_state.pin_a, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        return ret;
    }
    
    ret = gpio_pin_configure(enc_state.port_b, enc_state.pin_b, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        return ret;
    }
    
    ret = gpio_pin_configure(enc_state.port_btn, enc_state.pin_btn, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        return ret;
    }
    
    // Initialize encoder state
    enc_state.position = 0;
    enc_state.button_count = 0;
    enc_state.button_state = false;
    enc_state.button_time = 0;
    enc_state.timer_count = 0;
    enc_state.prev_enc_state = 0;
    enc_state.afilt = 0;
    enc_state.bfilt = 0;
    enc_state.enc_switch = 0;
    enc_state.enc_prev_button = 0;
    
    // Initialize timer for encoder processing
    k_timer_init(&enc_state.timer, encoder_timer_callback, NULL);
    int32_t period = ENCODER_TIMER_PERIOD_US;
    k_timer_start(&enc_state.timer, K_USEC(period), K_USEC(period));
    
    return 0;
}

bool encoder_has_events(void)
{
    static int32_t last_pos = 0;
    static int32_t last_btn_count = 0;
    
    bool pos_changed = (enc_state.position != last_pos);
    bool btn_changed = (enc_state.button_count != last_btn_count);
    
    if (pos_changed || btn_changed) {
        last_pos = enc_state.position;
        last_btn_count = enc_state.button_count;
        return true;
    }
    
    return false;
}

void encoder_process_events(void)
{
    // Events are processed in the timer callback
    // This function can be used for additional processing if needed
}

int32_t encoder_position(void)
{
    return enc_state.position;
}

int32_t encoder_button_count(void)
{
    return enc_state.button_count;
}

bool encoder_button_state(void)
{
    return enc_state.button_state;
}

int32_t encoder_button_time_ms(void)
{
    return (enc_state.button_time * ENCODER_TIMER_PERIOD_US) / 1000;
}

void encoder_reset(void)
{
    enc_state.position = 0;
    enc_state.button_count = 0;
    enc_state.button_time = 0;
    enc_state.timer_count = 0;
}

void encoder_set_position(int32_t value)
{
    enc_state.position = value;
}