/*
 * Reservoir monitoring for diyPresso
 * Ported to Zephyr RTOS
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "reservoir.h"
#include "hx711.h"  /* Assuming HX711 driver is available */

LOG_MODULE_REGISTER(reservoir, CONFIG_DIYPRESSO_LOG_LEVEL);

/* Configuration from Kconfig */
#define RESERVOIR_CAPACITY CONFIG_DIYPRESSO_RESERVOIR_CAPACITY
#define RESERVOIR_EMPTY_LEVEL (CONFIG_DIYPRESSO_RESERVOIR_EMPTY_LEVEL / 100.0)  /* Convert from hundredths to percentage */

/* Static state */
static struct reservoir_state res_state = {
    .level = 0.0,
    .tare = 0.0,
    .weight = 0.0,
    .offset = 240000.0,
    .scale = 427.4,
    .trim = 0.0,
    .readings = 0,
    .error = RESERVOIR_ERROR_NONE
};

/**
 * @brief Read current weight from HX711 sensor
 */
static void reservoir_read(void)
{
    int ret;
    long raw_weight;

    /* Get raw reading from HX711 */
    ret = hx711_read(&raw_weight);
    if (ret != 0) {
        res_state.error = RESERVOIR_ERROR_SENSOR;
        res_state.readings++;
        return;
    }

    /* Convert raw reading to actual weight */
    double net_weight = (raw_weight - res_state.offset) / res_state.scale;
    
    /* Apply trim */
    net_weight *= (1.0 + res_state.trim / 100.0);
    
    /* Store the reading */
    res_state.weight = net_weight + res_state.tare;
    
    /* Calculate level */
    res_state.level = MIN(100.0, MAX(0, 100.0 * (res_state.weight - res_state.tare) / RESERVOIR_CAPACITY));
    
    /* Clear error if successful */
    res_state.error = RESERVOIR_ERROR_NONE;
    res_state.readings = 0;
}

/**
 * @brief Initialize reservoir monitoring
 */
int reservoir_init(void)
{
    LOG_INF("Initializing reservoir monitoring");

    /* Initialize HX711 if not already done */
    int ret = hx711_init();
    if (ret < 0) {
        LOG_ERR("Failed to initialize HX711: %d", ret);
        return ret;
    }

    /* Take initial reading to establish baseline */
    reservoir_read();

    LOG_INF("Reservoir monitoring initialized");
    return 0;
}

/**
 * @brief Get current reservoir level in percentage
 */
double reservoir_level(void)
{
    reservoir_read();
    return res_state.level;
}

/**
 * @brief Get current reservoir weight in grams
 */
double reservoir_weight(void)
{
    reservoir_read();
    return res_state.weight - res_state.tare;  /* Return net weight */
}

/**
 * @brief Get tare value
 */
double reservoir_get_tare(void)
{
    return res_state.tare;
}

/**
 * @brief Set tare value
 */
void reservoir_set_tare(double tare)
{
    res_state.tare = tare;
    res_state.error = RESERVOIR_ERROR_NONE;
}

/**
 * @brief Set trim value
 */
void reservoir_set_trim(double trim)
{
    res_state.trim = trim;
}

/**
 * @brief Tare the reservoir (assuming it's full)
 */
void reservoir_tare(void)
{
    reservoir_read();
    res_state.tare = res_state.weight - RESERVOIR_CAPACITY;
    res_state.error = RESERVOIR_ERROR_NONE;
}

/**
 * @brief Check if reservoir is empty
 */
bool reservoir_is_empty(void)
{
    return reservoir_level() < RESERVOIR_EMPTY_LEVEL;
}

/**
 * @brief Check if reservoir is almost empty
 */
bool reservoir_is_almost_empty(void)
{
    return reservoir_level() < RESERVOIR_ALMOST_EMPTY_WARNING_LEVEL;
}

/**
 * @brief Check if there's an error
 */
bool reservoir_is_error(void)
{
    return res_state.error != RESERVOIR_ERROR_NONE;
}

/**
 * @brief Get error type
 */
reservoir_error_t reservoir_error(void)
{
    return res_state.error;
}

/**
 * @brief Get error text description
 */
const char *reservoir_get_error_text(void)
{
    switch (res_state.error) {
        case RESERVOIR_ERROR_NONE:
            return "No Error";
        case RESERVOIR_ERROR_SENSOR:
            return "Sensor Error";
        case RESERVOIR_ERROR_NO_READINGS:
            return "No Readings";
        case RESERVOIR_ERROR_OUT_OF_RANGE:
            return "Out of Range";
        case RESERVOIR_ERROR_NEGATIVE:
            return "Negative Reading";
        default:
            return "Unknown Error";
    }
}

/**
 * @brief Clear error state
 */
void reservoir_clear_error(void)
{
    res_state.error = RESERVOIR_ERROR_NONE;
}