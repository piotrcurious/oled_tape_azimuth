#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <cmath>

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define A0 0

typedef uint8_t byte;
typedef bool boolean;

long map(long x, long in_min, long in_max, long out_min, long out_max);

class __FlashStringHelper;

class MockSerial {
public:
    void begin(unsigned long baud) {}
    void print(const char* s) { std::cout << s; }
    void print(int i) { std::cout << i; }
    void print(unsigned long l) { std::cout << l; }
    void print(const __FlashStringHelper* s) { std::cout << (const char*)s; }
    void println(const char* s) { std::cout << s << std::endl; }
    void println(int i) { std::cout << i << std::endl; }
    void println(unsigned long l) { std::cout << l << std::endl; }
    void println(const __FlashStringHelper* s) { std::cout << (const char*)s << std::endl; }
};

extern MockSerial Serial;

unsigned long millis();
void delay(unsigned long ms);

int analogRead(int pin);
unsigned long pulseIn(int pin, int state, unsigned long timeout = 1000000L);
void pinMode(int pin, int mode);
bool digitalRead(int pin);

#define F(s) (const __FlashStringHelper*)(s)

// Simulation controls
extern int simulated_analog_value;
extern unsigned long simulated_pulse_length;
extern bool simulated_button_state;
extern unsigned long virtual_millis;

#endif
