# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(diypresso_stm32 VERSION 1.0.0)

target_sources(app PRIVATE
    src/main.c
    src/encoder.c
    src/display.c
    src/temperature.c
    src/hx711.c
    src/heater.c
    src/pump.c
    src/reservoir.c
    src/boiler.c
    src/menu.c
    src/settings.c
    src/pid.c
    src/brew.c
    src/wifi.c
    src/mqtt.c
)

target_include_directories(app PRIVATE include)