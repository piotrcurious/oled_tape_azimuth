CXX = g++
CXXFLAGS = -I test/mock -DARDUINO=100 -D__AVR_ATmega328P__ -std=c++11

SRCS = test/test_runner.cpp test/mock/Arduino.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = test/test_runner
SCREENSHOT_DIR = test/screenshots

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^

clean:
	rm -f $(OBJS) $(TARGET)
	rm -f $(SCREENSHOT_DIR)/*.ppm

test: all
	mkdir -p $(SCREENSHOT_DIR)
	./$(TARGET)

.PHONY: all clean test
