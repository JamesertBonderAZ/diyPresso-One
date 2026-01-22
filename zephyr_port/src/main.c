/*
 * diyPresso main controller for STM32L031K6
 * Ported to Zephyr RTOS
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#include "encoder.h"
#include "display.h"
#include "temperature.h"
#include "hx711.h"
#include "heater.h"
#include "pump.h"
#include "reservoir.h"
#include "boiler.h"
#include "menu.h"
#include "settings.h"
#include "brew.h"
#include "wifi.h"
#include "mqtt.h"

/* Stack definition for main thread */
#define MAIN_STACK_SIZE 2048
#define MAIN_PRIORITY 5

/* Global state structure */
struct diypresso_state {
    struct k_work_q work_q;
    struct k_thread work_q_thread;
    k_tid_t work_q_tid;
    struct k_sem sync_sem;
};

static struct diypresso_state app_state;

/* Work definitions for various subsystems */
static struct k_work encoder_work;
static struct k_work boiler_control_work;
static struct k_work brew_process_work;
static struct k_work display_update_work;
static struct k_work mqtt_send_work;

/* Thread stack for work queue */
K_THREAD_STACK_DEFINE(work_q_stack, 2048);

/* Forward declarations */
static void encoder_work_handler(struct k_work *work);
static void boiler_control_handler(struct k_work *work);
static void brew_process_handler(struct k_work *work);
static void display_update_handler(struct k_work *work);
static void mqtt_send_handler(struct k_work *work);

/**
 * @brief Initialize all subsystems
 */
static int initialize_subsystems(void)
{
    int ret;

    printk("Initializing diyPresso system...\n");

    /* Initialize GPIO subsystem */
    ret = gpio_init();
    if (ret < 0) {
        printk("GPIO initialization failed: %d\n", ret);
        return ret;
    }

    /* Initialize encoder */
    ret = encoder_init();
    if (ret < 0) {
        printk("Encoder initialization failed: %d\n", ret);
        return ret;
    }
    k_work_init(&encoder_work, encoder_work_handler);

    /* Initialize display */
    ret = display_init();
    if (ret < 0) {
        printk("Display initialization failed: %d\n", ret);
        return ret;
    }
    k_work_init(&display_update_work, display_update_handler);

    /* Initialize temperature sensor */
    ret = temperature_sensor_init();
    if (ret < 0) {
        printk("Temperature sensor initialization failed: %d\n", ret);
        return ret;
    }

    /* Initialize HX711 weight sensor */
    ret = hx711_init();
    if (ret < 0) {
        printk("HX711 initialization failed: %d\n", ret);
        return ret;
    }

    /* Initialize heater */
    ret = heater_init();
    if (ret < 0) {
        printk("Heater initialization failed: %d\n", ret);
        return ret;
    }

    /* Initialize pump */
    ret = pump_init();
    if (ret < 0) {
        printk("Pump initialization failed: %d\n", ret);
        return ret;
    }

    /* Initialize reservoir monitoring */
    ret = reservoir_init();
    if (ret < 0) {
        printk("Reservoir initialization failed: %d\n", ret);
        return ret;
    }

    /* Initialize boiler controller */
    ret = boiler_controller_init();
    if (ret < 0) {
        printk("Boiler controller initialization failed: %d\n", ret);
        return ret;
    }
    k_work_init(&boiler_control_work, boiler_control_handler);

    /* Initialize brew process */
    ret = brew_process_init();
    if (ret < 0) {
        printk("Brew process initialization failed: %d\n", ret);
        return ret;
    }
    k_work_init(&brew_process_handler, brew_process_handler);

    /* Initialize settings */
    ret = settings_load();
    if (ret < 0) {
        printk("Settings load failed: %d, loading defaults\n", ret);
        settings_defaults();
        settings_save();
    }

    /* Initialize WiFi */
    wifi_init();

    /* Initialize MQTT */
    mqtt_init();

    /* Initialize menu system */
    menu_init();

    printk("All subsystems initialized successfully\n");
    return 0;
}

/**
 * @brief Main application loop
 */
static void main_loop(void)
{
    static uint32_t loop_counter = 0;
    static uint32_t last_wifi_check = 0;
    static uint32_t last_mqtt_send = 0;

    while (1) {
        /* Process encoder input */
        if (encoder_has_events()) {
            k_work_submit_to_queue(&app_state.work_q, &encoder_work);
        }

        /* Update boiler control periodically */
        if ((loop_counter % 10) == 0) {  /* Every 100ms */
            k_work_submit_to_queue(&app_state.work_q, &boiler_control_work);
        }

        /* Update brew process periodically */
        if ((loop_counter % 5) == 0) {   /* Every 50ms */
            k_work_submit_to_queue(&app_state.work_q, &brew_process_work);
        }

        /* Update display periodically */
        if ((loop_counter % 20) == 0) {  /* Every 200ms */
            k_work_submit_to_queue(&app_state.work_q, &display_update_work);
        }

        /* Handle WiFi operations */
        if ((k_uptime_get_32() - last_wifi_check) > 1000) {  /* Every second */
            wifi_process();
            last_wifi_check = k_uptime_get_32();
        }

        /* Send MQTT updates periodically */
        if ((k_uptime_get_32() - last_mqtt_send) > 5000) {   /* Every 5 seconds */
            k_work_submit_to_queue(&app_state.work_q, &mqtt_send_work);
            last_mqtt_send = k_uptime_get_32();
        }

        loop_counter++;
        k_msleep(10);  /* 10ms tick */
    }
}

/* Work handlers */
static void encoder_work_handler(struct k_work *work)
{
    encoder_process_events();
}

static void boiler_control_handler(struct k_work *work)
{
    boiler_control();
}

static void brew_process_handler(struct k_work *work)
{
    brew_process_run();
}

static void display_update_handler(struct k_work *work)
{
    menu_update_display();
}

static void mqtt_send_handler(struct k_work *work)
{
    mqtt_send_state();
}

/**
 * @brief Main function
 */
void main(void)
{
    int ret;

    printk("Starting diyPresso on STM32L031K6\n");
    printk("Version: %s\n", CONFIG_DIYPRESSO_VERSION);

    /* Initialize the synchronization semaphore */
    k_sem_init(&app_state.sync_sem, 0, 1);

    /* Initialize work queue */
    k_work_queue_start(&app_state.work_q, work_q_stack,
                      K_THREAD_STACK_SIZEOF(work_q_stack),
                      K_PRIO_COOP(7), NULL);
    
    /* Initialize all subsystems */
    ret = initialize_subsystems();
    if (ret < 0) {
        printk("System initialization failed: %d\n", ret);
        return;
    }

    /* Start the work queue thread */
    k_thread_start(app_state.work_q_tid);

    /* Run the main application loop */
    main_loop();
}