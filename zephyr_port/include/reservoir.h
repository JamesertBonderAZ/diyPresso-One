/*
 * Reservoir monitoring for diyPresso
 * Ported to Zephyr RTOS
 */

#ifndef RESERVOIR_H_
#define RESERVOIR_H_

#include <zephyr/kernel.h>

/* Reservoir constants */
#define RESERVOIR_ALMOST_EMPTY_WARNING_LEVEL 12.0  /* Empty level threshold [%], triggers a warning to refill upon brew start */

/* Reservoir error types */
typedef enum {
    RESERVOIR_ERROR_NONE,
    RESERVOIR_ERROR_SENSOR,
    RESERVOIR_ERROR_NO_READINGS,
    RESERVOIR_ERROR_OUT_OF_RANGE,
    RESERVOIR_ERROR_NEGATIVE
} reservoir_error_t;

/* Reservoir state structure */
struct reservoir_state {
    double level;                 /* Level [0..100%] */
    double tare;                  /* Tare weight (when empty) [gr] */
    double weight;                /* Current gross weight [gr] */
    double offset;                /* Zero level offset [adc_units] */
    double scale;                 /* Scale [adc_units/gram] */
    double trim;                  /* Scale trim to match calibrated weight [%] */
    int readings;                 /* Number of readings without measurement */
    reservoir_error_t error;      /* Current error state */
};

/* Public API */
int reservoir_init(void);
double reservoir_level(void);
double reservoir_weight(void);
double reservoir_get_tare(void);
void reservoir_set_tare(double tare);
void reservoir_set_trim(double trim);
void reservoir_tare(void);
bool reservoir_is_empty(void);
bool reservoir_is_almost_empty(void);
bool reservoir_is_error(void);
reservoir_error_t reservoir_error(void);
const char *reservoir_get_error_text(void);
void reservoir_clear_error(void);

#endif /* RESERVOIR_H_ */