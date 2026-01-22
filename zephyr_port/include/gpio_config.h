/*
 * GPIO configuration for diyPresso STM32L031K6
 */

#ifndef GPIO_CONFIG_H_
#define GPIO_CONFIG_H_

#include <zephyr/device.h>

/* GPIO Pin Definitions for STM32L031K6 */
#define ENC_BTN_GPIO_PORT     DT_ALIAS(enc_btn_gpio_port)
#define ENC_BTN_GPIO_PIN      DT_GPIO_PIN(DT_ALIAS(enc_btn_gpio_port), gpios)

#define BREW_SW_GPIO_PORT     DT_ALIAS(brew_sw_gpio_port)
#define BREW_SW_GPIO_PIN      DT_GPIO_PIN(DT_ALIAS(brew_sw_gpio_port), gpios)

#define SSR_HEATER_GPIO_PORT  DT_ALIAS(ssr_heater_gpio_port)
#define SSR_HEATER_GPIO_PIN   DT_GPIO_PIN(DT_ALIAS(ssr_heater_gpio_port), gpios)

#define SSR_PUMP_GPIO_PORT    DT_ALIAS(ssr_pump_gpio_port)
#define SSR_PUMP_GPIO_PIN     DT_GPIO_PIN(DT_ALIAS(ssr_pump_gpio_port), gpios)

#define ENC_A_GPIO_PORT       DT_ALIAS(enc_a_gpio_port)
#define ENC_A_GPIO_PIN        DT_GPIO_PIN(DT_ALIAS(enc_a_gpio_port), gpios)

#define ENC_B_GPIO_PORT       DT_ALIAS(enc_b_gpio_port)
#define ENC_B_GPIO_PIN        DT_GPIO_PIN(DT_ALIAS(enc_b_gpio_port), gpios)

#define HX711_CLK_GPIO_PORT   DT_ALIAS(hx711_clk_gpio_port)
#define HX711_CLK_GPIO_PIN    DT_GPIO_PIN(DT_ALIAS(hx711_clk_gpio_port), gpios)

#define HX711_DAT_GPIO_PORT   DT_ALIAS(hx711_dat_gpio_port)
#define HX711_DAT_GPIO_PIN    DT_GPIO_PIN(DT_ALIAS(hx711_dat_gpio_port), gpios)

#define THERM_RDY_GPIO_PORT   DT_ALIAS(therm_rdy_gpio_port)
#define THERM_RDY_GPIO_PIN    DT_GPIO_PIN(DT_ALIAS(therm_rdy_gpio_port), gpios)

#define THERM_CS_GPIO_PORT    DT_ALIAS(therm_cs_gpio_port)
#define THERM_CS_GPIO_PIN     DT_GPIO_PIN(DT_ALIAS(therm_cs_gpio_port), gpios)

/* Helper macros */
#define GPIO_GET(dev, pin) gpio_pin_get(dev, pin)
#define GPIO_SET(dev, pin, value) gpio_pin_set(dev, pin, value)

/* Function prototypes */
int gpio_init(void);

#endif /* GPIO_CONFIG_H_ */