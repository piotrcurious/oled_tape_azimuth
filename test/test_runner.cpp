#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include "mock/Arduino.h"
#include "mock/Adafruit_SSD1306.h"

extern int simulated_analog_value;
extern unsigned long simulated_pulse_length;
extern bool simulated_button_state;
extern unsigned long virtual_millis;

#include "../head_azimuth.ino"

void generate_signals(int base_pulse, int jitter, int count) {
    for (int i = 0; i < count; i++) {
        virtual_millis += 10;
        // Jitter between -jitter and +jitter
        int current_jitter = (jitter > 0) ? (rand() % (jitter * 2 + 1)) - jitter : 0;
        simulated_pulse_length = base_pulse + current_jitter;
        loop();
    }
}

void press_button() {
    simulated_button_state = LOW;
    virtual_millis += 10;
    loop();
    simulated_button_state = HIGH;
    virtual_millis += 500;
    loop();
}

void take_screenshot(const std::string& name) {
    std::string filename = "test/screenshots/" + name + ".ppm";
    display.savePPM(filename);
    std::cout << "Saved screenshot: " << filename << std::endl;
}

int main() {
    srand(42); // Deterministic test
    setup();

    // 1. Splash screen
    take_screenshot("01_splash");
    virtual_millis += 3000; // Skip splash and initial message
    loop();

    // --- GOOD AZIMUTH SCENARIOS ---
    std::cout << "Testing Good Azimuth...\n";
    generate_signals(396, 2, 100); // 396us (Medium) with low jitter

    take_screenshot("02_histogram_good");

    press_button(); // to Head Fit
    take_screenshot("03_headfit_good");

    press_button(); // to Meter
    take_screenshot("04_meter_good");

    press_button(); // to Stats
    take_screenshot("05_stats_good");

    // --- BAD AZIMUTH SCENARIOS ---
    std::cout << "Testing Bad Azimuth...\n";
    press_button(); // Back to Histogram
    generate_signals(396, 40, 100); // 396us with high jitter

    take_screenshot("06_histogram_bad");

    press_button(); // to Head Fit
    take_screenshot("07_headfit_bad");

    press_button(); // to Meter
    take_screenshot("08_meter_bad");

    press_button(); // to Stats
    take_screenshot("09_stats_bad");

    return 0;
}
