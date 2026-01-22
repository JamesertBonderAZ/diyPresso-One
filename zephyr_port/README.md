# diyPresso Zephyr OS Port for STM32L031K6

This directory contains the Zephyr RTOS port of the diyPresso coffee machine controller for the STM32L031K6 microcontroller (STM Nucleo MB1180 Rev C).

## Overview

The diyPresso project is being ported from Arduino (MKR WiFi 1010) to Zephyr RTOS running on the STM32L031K6. This port maintains the core functionality while adapting to Zephyr's architecture and the STM32L031K6's hardware capabilities.

## Hardware Compatibility

### Pin Mapping
The following table shows the pin mapping from the original MKR WiFi 1010 to the STM32L031K6:

| Function | Original Pin | STM32L031K6 Pin | Purpose |
|----------|--------------|-----------------|---------|
| Rotary Encoder Button | 0 | PA0 | Push button input |
| Brew Switch | 1 | PA1 | Brew switch detection |
| Pump SSR Control | 2 | PA2 | Pump solid state relay |
| Heater SSR Control | 3 | PA3 | Heater solid state relay |
| HX711 Clock | 4 | PA4 | Weight sensor clock |
| HX711 Data | 5 | PA5 | Weight sensor data |
| Thermocouple Ready | 6 | PA6 | MAX31865 ready signal |
| Thermocouple Chip Select | 7 | PA7 | MAX31865 chip select |
| SPI MOSI | 8 | PA12 | SPI master out slave in |
| SPI SCK | 9 | PA11 | SPI serial clock |
| SPI MISO | 10 | PA10 | SPI master in slave out |
| I2C SDA | 11 | PB7 | I2C data line |
| I2C SCL | 12 | PB6 | I2C clock line |
| Rotary Encoder B | 13 | PB1 | Quadrature encoder B |
| Rotary Encoder A | 14 | PB0 | Quadrature encoder A |

### Peripherals Support
- **Temperature Sensor**: MAX31865 with PT1000 RTD (SPI interface)
- **Display**: 4x20 Character LCD (I2C interface, address 0x27)
- **User Interface**: Rotary encoder with push button
- **Weight Sensor**: HX711 Load Cell Amplifier
- **Actuators**: 2 Solid State Relays (Heater, Pump)

## Memory Constraints
The STM32L031K6 has 32KB Flash and 8KB RAM. The port is designed with these constraints in mind:
- Optimized data structures
- Minimal heap usage
- Efficient algorithms
- Selective feature inclusion

## Building Instructions

1. **Install Zephyr SDK and dependencies**
   ```bash
   # Follow Zephyr getting started guide to install the SDK
   # https://docs.zephyrproject.org/latest/getting_started/index.html
   ```

2. **Set up the development environment**
   ```bash
   source ~/zephyrproject/zephyr/zephyr-env.sh
   west update
   ```

3. **Build the application**
   ```bash
   cd /workspace/zephyr_port
   cmake -B build -DBOARD=nucleo_l031k6
   cmake --build build
   ```

4. **Flash to the board**
   ```bash
   west flash -d build
   ```

## Key Changes from Arduino Version

1. **RTOS Architecture**: Using Zephyr's threading and workqueue system instead of Arduino's single-threaded loop()
2. **Device Tree**: Hardware configuration through devicetree instead of hardcoded pin numbers
3. **Memory Management**: More efficient memory usage considering limited RAM
4. **Driver Integration**: Proper Zephyr driver integration for peripherals

## Limitations and Considerations

1. **WiFi Connectivity**: The STM32L031K6 does not have built-in WiFi. An external WiFi module (like ESP8266/ESP32) would be needed for WiFi functionality.
2. **PWM Implementation**: Using software PWM for heater control due to limited hardware PWM channels.
3. **Real-time Performance**: The system maintains real-time control requirements for heater safety.

## Status

This port provides the foundational architecture for diyPresso on STM32L031K6. Additional peripheral drivers and features may need to be implemented based on specific hardware configurations.

## Files Structure

- `src/` - Source files for all components
- `include/` - Header files
- `dts/` - Device tree overlays
- `CMakeLists.txt` - Build configuration
- `prj.conf` - Kconfig configuration
- `Kconfig` - Custom configuration options