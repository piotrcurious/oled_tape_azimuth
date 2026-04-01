# OLED Tape Azimuth Tool

Improved tape azimuth tool for Arduino (ATmega328P based) with 128x32 OLED display.
Inspired by Commodore 64 head alignment tools.

## Features
- Multiple modes switchable with a push button (Pin 2).
- **Scroll Mode**: FSK Pulse Length Plotter.
- **Head Fit Mode**: Visual alignment guide using vertical bars.
- **Meter Mode**: Dual bar meter for signal density and jitter quality.

## Pinout
- **TAPE_PIN**: A0 (Analog input for tape signal)
- **BUTTON_PIN**: D2 (Button to switch modes, pull to GND)
- **OLED (I2C)**:
  - SDA -> A4
  - SCL -> A5
  - Address: 0x3C

## Test Environment
The project includes a mock Arduino environment for testing on a PC.
To run the tests:
```bash
make test
```
This will compile the test runner and simulate different tape signals (good vs jittery) and show the resulting OLED buffer in the console.

## Credits
Based on FSK Pulse Length Plotter.
