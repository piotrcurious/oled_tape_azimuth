#ifndef ADAFRUIT_SSD1306_H
#define ADAFRUIT_SSD1306_H

#include "Arduino.h"
#include "Wire.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#define SSD1306_WHITE 1
#define SSD1306_BLACK 0
#define SSD1306_INVERSE 2

#define SSD1306_SWITCHCAPVCC 0x02

class Adafruit_GFX {
public:
    Adafruit_GFX(int16_t w, int16_t h) : _width(w), _height(h), cursor_x(0), cursor_y(0) {}
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    void setTextSize(uint8_t s) {}
    void setTextColor(uint16_t c) {}
    void setCursor(int16_t x, int16_t y) { cursor_x = x; cursor_y = y; }
    void print(const char* s) { mock_print(s); }
    void print(int i) { mock_print(std::to_string(i).c_str()); }
    void print(unsigned long l) { mock_print(std::to_string(l).c_str()); }
    void print(const __FlashStringHelper* s) { mock_print((const char*)s); }
    void println(const char* s) { mock_print(s); mock_print("\n"); }
    void println(const __FlashStringHelper* s) { mock_print((const char*)s); mock_print("\n"); }

    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
        if (x0 == x1) {
            for (int y = std::min(y0, y1); y <= std::max(y0, y1); y++) drawPixel(x0, y, color);
        } else if (y0 == y1) {
            for (int x = std::min(x0, x1); x <= std::max(x0, x1); x++) drawPixel(x, y0, color);
        } else {
            // Simple line algorithm for other cases (e.g. Bresenham if needed, but simple is okay for mock)
            drawPixel(x0, y0, color);
            drawPixel(x1, y1, color);
        }
    }
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
        for(int i=0; i<w; i++) {
            for(int j=0; j<h; j++) {
                drawPixel(x+i, y+j, color);
            }
        }
    }

    void mock_print(const char* s) {
        for (int i = 0; s[i] != '\0'; i++) {
            if (s[i] == '\n') {
                cursor_y += 8;
                cursor_x = 0;
            } else {
                // Mock text as simple pixel patterns
                drawPixel(cursor_x, cursor_y, SSD1306_WHITE);
                drawPixel(cursor_x + 1, cursor_y + 1, SSD1306_WHITE);
                cursor_x += 6;
            }
        }
    }

protected:
    int16_t _width, _height;
    int16_t cursor_x, cursor_y;
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

    void savePPM(const std::string& filename) {
        std::ofstream ofs(filename, std::ios::binary);
        if (!ofs) {
            std::cerr << "Failed to open " << filename << " for writing\n";
            return;
        }
        // P6: binary RGB, P3: ASCII RGB. P3 is easier to read/debug but P6 is more standard for tools.
        // Let's use P3 (ASCII) for simplicity and visibility.
        ofs << "P3\n" << _width << " " << _height << "\n255\n";
        for (int y = 0; y < _height; ++y) {
            for (int x = 0; x < _width; ++x) {
                uint8_t val = buffer[y * _width + x] ? 255 : 0;
                ofs << (int)val << " " << (int)val << " " << (int)val << " ";
            }
            ofs << "\n";
        }
        ofs.close();
    }

    std::vector<uint8_t> buffer;
};

#endif
