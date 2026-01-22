/*
 * HX711 Load Cell Amplifier for diyPresso
 * Ported to Zephyr RTOS
 */

#ifndef HX711_H_
#define HX711_H_

#include <zephyr/kernel.h>

/* HX711 error types */
typedef enum {
    HX711_ERROR_NONE,
    HX711_ERROR_INIT,
    HX711_ERROR_READ,
    HX711_ERROR_TIMEOUT
} hx711_error_t;

/* HX711 gain options */
#define HX711_GAIN_CHANNEL_A_128  0  /* Channel A, gain factor 128 */
#define HX711_GAIN_CHANNEL_A_64   1  /* Channel A, gain factor 64 */
#define HX711_GAIN_CHANNEL_B_32   2  /* Channel B, gain factor 32 */

/* Public API */
int hx711_init(void);
int hx711_read(long *result);
int hx711_set_gain(uint8_t gain);
int hx711_power_down(void);
int hx711_power_up(void);
bool hx711_is_ready(void);
bool hx711_is_error(void);
hx711_error_t hx711_get_error(void);
const char *hx711_get_error_text(void);
void hx711_clear_error(void);

#endif /* HX711_H_ */