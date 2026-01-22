/*
 * Display interface for diyPresso
 * Ported to Zephyr RTOS
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <zephyr/kernel.h>

/* Display error types */
typedef enum {
    DISPLAY_ERROR_NONE,
    DISPLAY_ERROR_INIT,
    DISPLAY_ERROR_COMM,
    DISPLAY_ERROR_BUSY
} display_error_t;

/* Display dimensions */
#define DISPLAY_COLS 20
#define DISPLAY_ROWS 4

/* Public API */
int display_init(void);
int display_print(const char *str, int row, int col);
int display_clear(void);
int display_clear_row(int row);
int display_set_cursor(int row, int col);
int display_backlight_on(void);
int display_backlight_off(void);
bool display_is_error(void);
display_error_t display_get_error(void);
const char *display_get_error_text(void);
void display_clear_error(void);

#endif /* DISPLAY_H_ */