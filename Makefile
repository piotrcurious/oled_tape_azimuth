CXX = g++
CXXFLAGS = -I test/mock -DARDUINO=100 -D__AVR_ATmega328P__ -std=c++11

SRCS = test/test_runner.cpp test/mock/Arduino.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = test/test_runner

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^

clean:
	rm -f $(OBJS) $(TARGET)

test: all
	./$(TARGET)

.PHONY: all clean test
