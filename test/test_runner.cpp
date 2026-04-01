#include <iostream>
#include <vector>
#include <cstring>
#include "mock/Arduino.h"
#include "mock/Adafruit_SSD1306.h"

extern int simulated_analog_value;
extern unsigned long simulated_pulse_length;
extern bool simulated_button_state;
extern unsigned long virtual_millis;

#include "../head_azimuth.ino"

void print_buffer(Adafruit_SSD1306& display, const char* title) {
    std::cout << "\n--- " << title << " ---\n";
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 128; x++) {
            std::cout << (display.buffer[y * 128 + x] ? '#' : '.');
        }
        std::cout << std::endl;
    }
}

void next_mode() {
    simulated_button_state = LOW;
    virtual_millis += 10;
    loop();
    simulated_button_state = HIGH;
    virtual_millis += 600;
    loop();
}

int main() {
    setup();

    // Simulate some realistic pulses around C64_MEDIUM (396 us) with some jitter
    for(int i = 0; i < 200; i++) {
        virtual_millis += 10;
        // Jitter: +/- 10us
        simulated_pulse_length = 396 + (i % 21) - 10;
        loop();
    }

    print_buffer(display, "HISTOGRAM MODE");

    next_mode(); // to Head Fit
    print_buffer(display, "HEAD FIT MODE");

    next_mode(); // to Meter
    print_buffer(display, "METER MODE");

    next_mode(); // to Stats
    print_buffer(display, "STATS MODE");

    return 0;
}
