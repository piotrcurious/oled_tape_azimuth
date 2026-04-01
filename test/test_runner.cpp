#include <iostream>
#include <vector>
#include "mock/Arduino.h"
#include "mock/Adafruit_SSD1306.h"

// Define these to simulate the environment
extern int simulated_analog_value;
extern unsigned long simulated_pulse_length;
extern bool simulated_button_state;
extern unsigned long virtual_millis;

#include "../../head_azimuth.ino"

void print_buffer(Adafruit_SSD1306& display) {
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 128; x++) {
            std::cout << (display.buffer[y * 128 + x] ? '#' : '.');
        }
        std::cout << std::endl;
    }
}

int main() {
    setup();

    // Simulate some jittery pulses (e.g. bad azimuth)
    simulated_analog_value = 600;

    std::cout << "--- Simulating High Jitter (Bad Azimuth) ---" << std::endl;
    for(int i = 0; i < 100; i++) {
        virtual_millis += 10;
        simulated_pulse_length = 350 + (i % 50); // Jitter between 350 and 400
        loop();
    }

    // Switch to Meter Mode
    simulated_button_state = LOW;
    virtual_millis += 10;
    loop();
    simulated_button_state = HIGH;
    virtual_millis += 600;
    loop();
    simulated_button_state = LOW;
    virtual_millis += 10;
    loop();
    simulated_button_state = HIGH;
    virtual_millis += 600;
    loop();

    std::cout << "Meter with High Jitter:" << std::endl;
    print_buffer(display);

    std::cout << "--- Simulating Low Jitter (Good Azimuth) ---" << std::endl;
    for(int i = 0; i < 100; i++) {
        virtual_millis += 10;
        simulated_pulse_length = 350 + (i % 2); // Very low jitter
        loop();
    }
    std::cout << "Meter with Low Jitter:" << std::endl;
    print_buffer(display);

    return 0;
}
