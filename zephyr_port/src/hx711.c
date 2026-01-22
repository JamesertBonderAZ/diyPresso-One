/*
 * HX711 Load Cell Amplifier for diyPresso
 * Ported to Zephyr RTOS
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "hx711.h"
#include "gpio_config.h"

LOG_MODULE_REGISTER(hx711, CONFIG_DIYPRESSO_LOG_LEVEL);

/* Pin configuration from hardware definitions */
#define HX711_SCK_GPIO DT_GPIO_PIN(DT_PATH(zephyr_user, hx711_sck), gpios)
#define HX711_DOUT_GPIO DT_GPIO_PIN(DT_PATH(zephyr_user, hx711_dout), gpios)

/* Device node references */
#define HX711_SCK_NODE DT_PATH(zephyr_user, hx711_sck)
#define HX711_DOUT_NODE DT_PATH(zephyr_user, hx711_dout)

/* Timing constants */
#define HX711_TIMEOUT_US 1000000  /* 1 second timeout for data ready */
#define HX711_DELAY_US 1          /* Delay between clock pulses */

/* Static variables */
static const struct device *sck_gpio_dev;
static const struct device *dout_gpio_dev;
static hx711_error_t hx711_error = HX711_ERROR_NONE;
static uint8_t current_gain = HX711_GAIN_CHANNEL_A_128;

/**
 * @brief Initialize HX711 module
 */
int hx711_init(void)
{
    LOG_INF("Initializing HX711");

    /* Get GPIO devices */
    sck_gpio_dev = DEVICE_DT_GET(DT_GPIO_CTLR(HX711_SCK_NODE, gpios));
    dout_gpio_dev = DEVICE_DT_GET(DT_GPIO_CTLR(HX711_DOUT_NODE, gpios));

    if (!device_is_ready(sck_gpio_dev)) {
        LOG_ERR("SCK GPIO device not ready");
        hx711_error = HX711_ERROR_INIT;
        return -ENODEV;
    }

    if (!device_is_ready(dout_gpio_dev)) {
        LOG_ERR("DOUT GPIO device not ready");
        hx711_error = HX711_ERROR_INIT;
        return -ENODEV;
    }

    /* Configure GPIO pins */
    int ret = gpio_pin_configure(sck_gpio_dev, HX711_SCK_GPIO, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure SCK pin: %d", ret);
        hx711_error = HX711_ERROR_INIT;
        return ret;
    }

    ret = gpio_pin_configure(dout_gpio_dev, HX711_DOUT_GPIO, GPIO_INPUT);
    if (ret < 0) {
        LOG_ERR("Failed to configure DOUT pin: %d", ret);
        hx711_error = HX711_ERROR_INIT;
        return ret;
    }

    /* Power up the HX711 by setting SCK low */
    gpio_pin_set(sck_gpio_dev, HX711_SCK_GPIO, 0);

    /* Set default gain */
    ret = hx711_set_gain(current_gain);
    if (ret != 0) {
        LOG_ERR("Failed to set default gain: %d", ret);
        hx711_error = HX711_ERROR_INIT;
        return ret;
    }

    LOG_INF("HX711 initialized successfully");
    hx711_error = HX711_ERROR_NONE;
    return 0;
}

/**
 * @brief Read a value from the HX711
 */
int hx711_read(long *result)
{
    if (!result) {
        return -EINVAL;
    }

    uint32_t timeout_count = 0;
    int ret;

    /* Wait for the data to be ready */
    while (gpio_pin_get(dout_gpio_dev, HX711_DOUT_GPIO) == 1) {
        k_usleep(10);
        timeout_count += 10;
        if (timeout_count > HX711_TIMEOUT_US) {
            LOG_ERR("HX711 read timeout");
            hx711_error = HX711_ERROR_TIMEOUT;
            return -ETIMEDOUT;
        }
    }

    /* Read 24 bits of data */
    long data = 0;
    for (int i = 0; i < 24; i++) {
        /* Toggle clock high then low */
        gpio_pin_set(sck_gpio_dev, HX711_SCK_GPIO, 1);
        k_usleep(HX711_DELAY_US);
        gpio_pin_set(sck_gpio_dev, HX711_SCK_GPIO, 0);
        
        /* Shift in the data bit */
        data <<= 1;
        if (gpio_pin_get(dout_gpio_dev, HX711_DOUT_GPIO) == 1) {
            data |= 1;
        }
        k_usleep(HX711_DELAY_US);
    }

    /* Set gain for next reading (25th-27th pulses) */
    for (int i = 0; i < 27 - 24; i++) {
        gpio_pin_set(sck_gpio_dev, HX711_SCK_GPIO, 1);
        k_usleep(HX711_DELAY_US);
        gpio_pin_set(sck_gpio_dev, HX711_SCK_GPIO, 0);
        k_usleep(HX711_DELAY_US);
    }

    /* Convert to signed 24-bit value */
    if (data & 0x800000) {
        data -= 0x1000000;
    }

    *result = data;
    hx711_error = HX711_ERROR_NONE;
    return 0;
}

/**
 * @brief Set gain for HX711
 */
int hx711_set_gain(uint8_t gain)
{
    int ret;
    current_gain = gain;

    /* To set gain, we need to perform a read with the appropriate number of pulses */
    /* We'll just ensure the next read uses the correct gain setting */
    
    /* Make sure device is powered up */
    ret = hx711_power_up();
    if (ret != 0) {
        hx711_error = HX711_ERROR_INIT;
        return ret;
    }

    /* Cycle clock the appropriate number of times to set gain */
    for (int i = 0; i < 27 - 24; i++) {
        gpio_pin_set(sck_gpio_dev, HX711_SCK_GPIO, 1);
        k_usleep(HX711_DELAY_US);
        gpio_pin_set(sck_gpio_dev, HX711_SCK_GPIO, 0);
        k_usleep(HX711_DELAY_US);
    }

    hx711_error = HX711_ERROR_NONE;
    return 0;
}

/**
 * @brief Power down HX711
 */
int hx711_power_down(void)
{
    /* Pull SCK high for at least 10 microseconds to power down */
    gpio_pin_set(sck_gpio_dev, HX711_SCK_GPIO, 1);
    k_usleep(100);
    
    hx711_error = HX711_ERROR_NONE;
    return 0;
}

/**
 * @brief Power up HX711
 */
int hx711_power_up(void)
{
    /* Pull SCK low to power up */
    gpio_pin_set(sck_gpio_dev, HX711_SCK_GPIO, 0);
    
    hx711_error = HX711_ERROR_NONE;
    return 0;
}

/**
 * @brief Check if HX711 is ready for reading
 */
bool hx711_is_ready(void)
{
    return gpio_pin_get(dout_gpio_dev, HX711_DOUT_GPIO) == 0;
}

/**
 * @brief Check if there's an error
 */
bool hx711_is_error(void)
{
    return hx711_error != HX711_ERROR_NONE;
}

/**
 * @brief Get error type
 */
hx711_error_t hx711_get_error(void)
{
    return hx711_error;
}

/**
 * @brief Get error text description
 */
const char *hx711_get_error_text(void)
{
    switch (hx711_error) {
        case HX711_ERROR_NONE:
            return "No Error";
        case HX711_ERROR_INIT:
            return "Initialization Error";
        case HX711_ERROR_READ:
            return "Read Error";
        case HX711_ERROR_TIMEOUT:
            return "Timeout Error";
        default:
            return "Unknown Error";
    }
}

/**
 * @brief Clear error state
 */
void hx711_clear_error(void)
{
    hx711_error = HX711_ERROR_NONE;
}