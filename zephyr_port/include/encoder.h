/*
 * Rotary encoder driver for diyPresso
 * Ported to Zephyr RTOS
 */

#ifndef ENCODER_H_
#define ENCODER_H_

#include <zephyr/kernel.h>

/* Encoder configuration */
#define ENCODER_TIMER_PERIOD_US 400  /* 400usec timer = 2.5kHz */
#define ENCODER_DEGLITCH_COUNT 2     /* Deglitch count value */

/* Button configuration */
#define BUTTON_DEGLITCH_BITS 0xFFFFFFF  /* Bits that need to be high before switching back */

/* Encoder state structure */
struct encoder_state {
    volatile int32_t position;
    volatile int32_t button_count;
    volatile bool button_state;
    volatile int32_t button_time;
    volatile int32_t timer_count;
    struct k_timer timer;
    struct gpio_callback gpio_cb_a;
    struct gpio_callback gpio_cb_b;
    struct gpio_callback gpio_cb_btn;
    const struct device *port_a;
    const struct device *port_b;
    const struct device *port_btn;
    uint32_t pin_a;
    uint32_t pin_b;
    uint32_t pin_btn;
    volatile uint32_t prev_enc_state;
    volatile uint32_t afilt;
    volatile uint32_t bfilt;
    volatile uint32_t enc_switch;
    volatile uint32_t enc_prev_button;
};

/* Public API */
int encoder_init(void);
bool encoder_has_events(void);
void encoder_process_events(void);
int32_t encoder_position(void);
int32_t encoder_button_count(void);
bool encoder_button_state(void);
int32_t encoder_button_time_ms(void);
void encoder_reset(void);
void encoder_set_position(int32_t value);

#endif /* ENCODER_H_ */