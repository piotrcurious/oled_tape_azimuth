#ifndef ADAFRUIT_SSD1306_H
#define ADAFRUIT_SSD1306_H

#include "Arduino.h"
#include <vector>

#define SSD1306_WHITE 1
#define SSD1306_BLACK 0
#define SSD1306_INVERSE 2

#define SSD1306_SWITCHCAPVCC 0x02

class Wire_Mock {
};

extern Wire_Mock Wire;

class Adafruit_GFX {
public:
    Adafruit_GFX(int16_t w, int16_t h) : _width(w), _height(h) {}
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    void setTextSize(uint8_t s) {}
    void setTextColor(uint16_t c) {}
    void setCursor(int16_t x, int16_t y) {}
    void print(const char* s) {}
    void print(int i) {}
    void print(const __FlashStringHelper* s) {}
    void println(const char* s) {}
    void println(const __FlashStringHelper* s) {}
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
        drawPixel(x0, y0, color);
        drawPixel(x1, y1, color);
    }
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
        for(int i=0; i<w; i++) {
            for(int j=0; j<h; j++) {
                drawPixel(x+i, y+j, color);
            }
        }
    }
protected:
    int16_t _width, _height;
};

class Adafruit_SSD1306 : public Adafruit_GFX {
public:
    Adafruit_SSD1306(uint8_t w, uint8_t h, Wire_Mock* wire = &Wire, int8_t rst = -1)
        : Adafruit_GFX(w, h), buffer(w * h, 0) {}

    bool begin(uint8_t switchvcc = SSD1306_SWITCHCAPVCC, uint8_t i2caddr = 0x3C, bool reset = true, bool periphBegin = true) {
        return true;
    }

    void clearDisplay() { std::fill(buffer.begin(), buffer.end(), 0); }
    void display() {}

    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        if (x >= 0 && x < _width && y >= 0 && y < _height) {
            buffer[y * _width + x] = color;
        }
    }

    std::vector<uint8_t> buffer;
};

#endif
