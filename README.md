# OLED Tape Azimuth Tool v3.0

Improved tape azimuth tool for Arduino (ATmega328P based) with 128x32 I2C OLED display.
Inspired by classic Commodore 64 head alignment utilities like "Head Fit".

## Features
- **Four Operating Modes**:
  - **HISTOGRAM**: Visual distribution of pulse lengths. Shows consistency across short, medium, and long pulses.
  - **HEAD FIT**: Scatter-plot alignment guide. Match vertical dots to top/bottom markers for perfect azimuth.
  - **METER**: Dual-bar display showing signal strength (data density) and stability (jitter quality).
  - **STATS**: Real-time numeric pulse data (Average, Min, Max, and Jitter in microseconds).
- **C64-Themed Splash Screen**: Start-up screen inspired by the Commodore 64's BASIC environment.
- **Calibrated for C64**: Precise reference markers for 284us (Short), 396us (Medium), and 508us (Long) pulses.
- **Physical Interaction**: Cycle through modes using a single push button.

## Visual Documentation

### Histogram Mode
Displays a frequency distribution of pulse lengths. Good azimuth shows sharp, narrow peaks at reference points.
| Good Signal | Bad Signal (High Jitter) |
|-------------|--------------------------|
| ![Histogram Good](assets/screenshots/02_histogram_good.png) | ![Histogram Bad](assets/screenshots/06_histogram_bad.png) |

### Head Fit Mode
Inspired by professional alignment tools. Azimuth is correct when dots form a straight vertical line under the markers.
| Good Alignment | Bad Alignment |
|----------------|---------------|
| ![Head Fit Good](assets/screenshots/03_headfit_good.png) | ![Head Fit Bad](assets/screenshots/07_headfit_bad.png) |

### Meter Mode
Quick visual reference for signal quality.
| Good Signal | Bad Signal |
|-------------|------------|
| ![Meter Good](assets/screenshots/04_meter_good.png) | ![Meter Bad](assets/screenshots/08_meter_bad.png) |

### Stats Mode
Detailed numeric analysis of the last 32 pulses.
| Statistics |
|------------|
| ![Stats](assets/screenshots/05_stats_good.png) |

## Pinout
- **TAPE_PIN**: D3 (Pulse input from tape interface)
- **BUTTON_PIN**: D2 (Mode select button, pull to GND)
- **OLED (I2C)**:
  - SDA -> A4
  - SCL -> A5
  - Address: 0x3C (Default)

## Test & Simulation Environment
The project includes a C++ mock Arduino environment to verify logic and generate screenshots without physical hardware.
To compile and run the simulation:
```bash
make test
```

## Credits
Developed for Commodore 64 enthusiasts. Based on FSK pulse measurement principles.
