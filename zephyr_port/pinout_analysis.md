# diyPresso System - Pinout Analysis and STM32L031K6 Porting Guide

## Current System Overview

### Hardware Components
- **Current Board**: Arduino MKR WiFi 1010 (SAMD21G18A)
- **Target Board**: STM32L031K6 (STM Nucleo MB1180 Rev C)
- **Memory**: 32KB Flash, 8KB RAM (STM32L031K6)

### Pin Assignments (Current MKR WiFi 1010)
| Pin Number | Function | Type | Description |
|------------|----------|------|-------------|
| 0 | PIN_ENC_S | Input | Rotary encoder push button |
| 1 | PIN_BREW_SWITCH | Input | Brew switch detection |
| 2 | PIN_SSR_PUMP | Output | Pump SSR control |
| 3 | PIN_SSR_HEATER | Output | Heater SSR control |
| 4 | PIN_HX711_CLK | Output | Weight sensor clock |
| 5 | PIN_HX711_DAT | Input | Weight sensor data |
| 6 | PIN_THERM_RDY | Input | Thermocouple ready signal |
| 7 | PIN_THERM_CS | Output | Thermocouple chip select |
| 8 | PIN_THERM_MOSI | Output | Thermocouple SPI MOSI |
| 9 | PIN_THEM_SCLK | Output | Thermocouple SPI clock |
| 10 | PIN_THERM_MISO | Input | Thermocouple SPI MISO |
| 11 | PIN_SDA | I2C Data | LCD display SDA |
| 12 | PIN_SCL | I2C Clock | LCD display SCL |
| 13 | PIN_ENC_B | Input | Rotary encoder B phase |
| 14 | PIN_ENC_A | Input | Rotary encoder A phase |

### Communication Protocols
- **SPI**: MAX31865 PT1000 temperature sensor (MOSI, MISO, SCLK, CS)
- **I2C**: 4x20 LCD display (SDA, SCL)
- **GPIO**: Switches, SSRs, encoder inputs
- **WiFi**: Built-in NINA-W102 module

### Peripherals Requirements
- **Temperature Sensor**: MAX31865 (SPI interface) with PT1000 RTD
- **Display**: 4x20 Character LCD (I2C interface)
- **User Interface**: Rotary encoder (2 quadrature pins + 1 button)
- **Weight Sensor**: HX711 Load Cell Amplifier (SPI-like interface)
- **Actuators**: 2 Solid State Relays (SSRs)

## STM32L031K6 Compatibility Analysis

### STM32L031K6 Features
- ARM Cortex-M0+ core
- 32 KB Flash memory
- 8 KB SRAM
- 48-pin LQFP package
- Multiple timers, ADC, SPI, I2C, UART peripherals
- GPIO ports A, B (some pins available)

### Pin Mapping Strategy for STM32L031K6
Based on the STM32L031K6 pinout, here's a proposed mapping:

| Function | Current Pin | Proposed STM32 Pin | STM32 Port/Pin | Notes |
|----------|-------------|-------------------|----------------|-------|
| PIN_ENC_S | 0 | PA0 | PA0 | Rotary encoder button |
| PIN_BREW_SWITCH | 1 | PA1 | PA1 | Brew switch input |
| PIN_SSR_PUMP | 2 | PA2 | PA2 | Pump SSR control |
| PIN_SSR_HEATER | 3 | PA3 | PA3 | Heater SSR control |
| PIN_HX711_CLK | 4 | PA4 | PA4 | Weight sensor clock |
| PIN_HX711_DAT | 5 | PA5 | PA5 | Weight sensor data (input) |
| PIN_THERM_RDY | 6 | PA6 | PA6 | Thermocouple ready (input) |
| PIN_THERM_CS | 7 | PA7 | PA7 | Thermocouple chip select |
| SPI_MOSI | 8 | PA12 | PA12 | SPI MOSI for thermocouple |
| SPI_SCK | 9 | PA11 | PA11 | SPI clock for thermocouple |
| SPI_MISO | 10 | PA10 | PA10 | SPI MISO for thermocouple |
| I2C_SDA | 11 | PB7 | PB7 | I2C data for LCD |
| I2C_SCL | 12 | PB6 | PB6 | I2C clock for LCD |
| PIN_ENC_B | 13 | PB1 | PB1 | Rotary encoder B phase |
| PIN_ENC_A | 14 | PB0 | PB0 | Rotary encoder A phase |

### STM32L031K6 Resource Availability
- **GPIO**: Sufficient pins available for all required functions
- **SPI**: 2 SPI interfaces available (SPI1, SPI2) - can handle both MAX31865 and HX711
- **I2C**: 2 I2C interfaces available (I2C1, I2C2) - sufficient for LCD display
- **Timers**: Multiple timers available for PWM and timing functions
- **ADC**: Available for future analog sensors (not currently needed)

## Memory Considerations
The original Arduino code needs to be adapted for Zephyr RTOS which has different memory requirements. With 32KB flash and 8KB RAM, we need to be conservative:

- Optimize data structures
- Remove unnecessary features
- Use efficient algorithms
- Consider static allocation over dynamic allocation

## Zephyr OS Implementation Plan

1. **Hardware Abstraction Layer**: Create device tree overlays for pin configuration
2. **Peripheral Drivers**: Implement drivers for MAX31865, HX711, LCD, encoder
3. **Application Structure**: Adapt the existing state machine architecture to Zephyr threading model
4. **Memory Optimization**: Minimize heap usage and optimize buffer sizes

## Potential Challenges
- WiFi connectivity: STM32L031K6 doesn't have built-in WiFi; need external module
- Different peripheral implementations in Zephyr vs Arduino
- Memory constraints requiring optimization
- Real-time requirements for heater control