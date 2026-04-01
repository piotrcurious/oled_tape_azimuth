# OLED Tape Azimuth Tool v3.0

Improved tape azimuth tool for Arduino (ATmega328P based) with 128x32 I2C OLED display.
Inspired by classic Commodore 64 head alignment utilities.

## Features
- **Four Operating Modes**:
  - **HISTOGRAM**: Visual distribution of pulse lengths.
  - **HEAD FIT**: Scatter-plot alignment guide (match vertical dots to markers).
  - **METER**: Signal strength and jitter stability bars.
  - **STATS**: Real-time numeric pulse data (Avg, Min, Max, Jitter).
- **C64-Themed Splash Screen**: Start-up screen inspired by the Commodore 64.
- **Improved Accuracy**: Calibrated for standard C64 pulse lengths (284, 396, 508 us).
- **Switchable Modes**: Toggle modes using a physical push button.

## Pinout
- **TAPE_PIN**: D3 (Pulse input from tape interface)
- **BUTTON_PIN**: D2 (Mode select button, pull to GND)
- **OLED (I2C)**:
  - SDA -> A4
  - SCL -> A5
  - Address: 0x3C (Default)

## Test Environment
This project includes a built-in mock Arduino environment for testing on a PC.
To compile and run the simulation:
```bash
make test
```
This will simulate various pulse signals and show the OLED buffer content in the console.

## Credits
Based on FSK Pulse Length Plotter and C64 "Head Fit" style tools.
