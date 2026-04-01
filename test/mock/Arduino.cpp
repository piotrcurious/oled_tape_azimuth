#include "Arduino.h"
#include "Wire.h"
#include "Adafruit_SSD1306.h"

long map(long x, long in_min, long in_max, long out_min, long out_max) {
    if (in_max == in_min) return out_min;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

MockSerial Serial;
Wire_Mock Wire;

int simulated_analog_value = 0;
unsigned long simulated_pulse_length = 0;
bool simulated_button_state = HIGH;
unsigned long virtual_millis = 0;

unsigned long millis() {
    return virtual_millis;
}

void delay(unsigned long ms) {
    virtual_millis += ms;
}

int analogRead(int pin) {
    return simulated_analog_value;
}

unsigned long pulseIn(int pin, int state, unsigned long timeout) {
    return simulated_pulse_length;
}

void pinMode(int pin, int mode) {}

bool digitalRead(int pin) {
    if (pin == 2) return simulated_button_state;
    return HIGH;
}
