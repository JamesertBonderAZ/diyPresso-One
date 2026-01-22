
# Espresso Controller for STM Nucleo MB1180 Rev C with Zephyr OS

This is a port of the DIY espresso machine controller to run on the STM Nucleo MB1180 Rev C board with Zephyr RTOS.

## Hardware Requirements

The system connects to various components:

### Actuators
- **PIN_BREW_SWITCH** (GPIO): Brew switch control
- **PIN_SSR_PUMP** (GPIO): Pump SSR relay control  
- **PIN_SSR_HEATER** (GPIO): Heater SSR relay control

### Sensors
- **HX711 Load Cell Amplifier**:
  - **PIN_HX711_CLK** (GPIO): Clock signal
  - **PIN_HX711_DAT** (GPIO): Data signal

- **MAX31865 PT1000 Temperature Amplifier**:
  - **PIN_THERM_RDY** (GPIO): Ready/Done indicator
  - **PIN_THERM_CS** (GPIO): Chip Select
  - **PIN_THERM_MOSI** (GPIO): Master Out Slave In
  - **PIN_THERM_SCLK** (GPIO): Serial Clock
  - **PIN_THERM_MISO** (GPIO): Master In Slave Out
  - Communication: SPI protocol

### Display and User Interface
- **LCD Display (4x20)**:
  - **PIN_LCD_SDA** (GPIO): I2C Data line
  - **PIN_LCD_SCL** (GPIO): I2C Clock line
  - Address: 0x27
  - Communication: I2C protocol

- **Rotary Encoder**:
  - **PIN_ENC_A** (GPIO): Encoder channel A
  - **PIN_ENC_B** (GPIO): Encoder channel B
  - **PIN_ENC_S** (GPIO): Encoder button switch
  - Communication: Quadrature encoder with button

## Building the Project

1. Install Zephyr SDK and set up the development environment
2. Navigate to this project directory
3. Build for the target board:

```bash
west build -p auto -b nucleo_f446re  # Adjust board name as needed
```

Or if using a custom board definition:

```bash
west build -p auto -b custom_stm32f4_board
```

4. Flash the application:

```bash
west flash
```

## Configuration

Pin assignments can be customized in the device tree overlay file `espresso_board.overlay`. Update the GPIO numbers according to your specific hardware connections.

## Features

- Temperature monitoring and control with PID algorithm
- Weight-based brewing with HX711 load cell
- Rotary encoder for user input
- LCD display showing system status
- Automated brewing process with pre-infusion
- Brewing timer and weight targets

## Customization

The following parameters can be adjusted in `src/espresso_config.h`:
- Temperature setpoints and PID constants
- Brewing times and weight targets
- Update intervals for various subsystems
- Calibration factors for sensors
