
// Include the Adafruit OLED library
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define the OLED display size and address
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_ADDR   0x3C // OLED display address

// Create an Adafruit OLED object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Define the digital input pin for the tape signal (using digital for pulseIn is better)
#define TAPE_PIN 3
// Define the digital input pin for the mode button
#define BUTTON_PIN 2

// Define the maximum number of pulses to store
#define MAX_PULSES 128 

// Define the time interval for updating the display (in milliseconds)
#define DISPLAY_INTERVAL 40

#define MIN_PULSE 200
#define MAX_PULSE 900

// Create an array to store the pulse lengths
unsigned long pulses[MAX_PULSES];

// Create a variable to store the current pulse index
int pulseIndex = 0;

// Create a variable to store the last display update time
unsigned long lastUpdateTime = 0;

// Mode definitions
enum Mode {
  MODE_SCROLL,    // FSK Pulse Length Plotter (original)
  MODE_HEADFIT,   // Consistency dots (similar to "Head Fit" tool)
  MODE_METER      // Signal quality bar meter
};

Mode currentMode = MODE_SCROLL;
bool lastButtonState = HIGH;

// Function prototypes for C++ compatibility
void drawScrollMode();
void drawHeadFitMode();
void drawMeterMode();

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Initialize pins
  pinMode(TAPE_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize the OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the display buffer
  display.clearDisplay();

  // Set the text size and color
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Display a welcome message
  display.setCursor(0,0);
  display.println(F("Tape Head Tool v2.1"));
  display.display();
  delay(1000);
}

void loop() {
  // Handle mode switching
  bool buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH) {
    currentMode = (Mode)((currentMode + 1) % 3);
    display.clearDisplay();
    display.setCursor(0,0);
    display.print(F("Mode: "));
    switch(currentMode) {
      case MODE_SCROLL:  display.println(F("Scroll")); break;
      case MODE_HEADFIT: display.println(F("Head Fit")); break;
      case MODE_METER:   display.println(F("Meter")); break;
    }
    display.display();
    delay(500); // debounce and show mode name
    lastUpdateTime = millis();
  }
  lastButtonState = buttonState;

  // Use pulseIn directly to measure pulse lengths
  // Wait for a HIGH pulse on the TAPE_PIN. Timeout of 10ms.
  unsigned long pulseLength = pulseIn(TAPE_PIN, HIGH, 10000);

  if (pulseLength > 0) {
    // Store the pulse length in the array
    pulses[pulseIndex] = pulseLength;
    // Increment the pulse index and wrap around if necessary
    pulseIndex = (pulseIndex + 1) % MAX_PULSES;
  }

  // Get the current time
  unsigned long currentTime = millis();

  // Update display based on mode
  if (currentTime - lastUpdateTime > DISPLAY_INTERVAL) {
    lastUpdateTime = currentTime;

    if (currentMode == MODE_SCROLL) {
      drawScrollMode();
    } else if (currentMode == MODE_HEADFIT) {
      drawHeadFitMode();
    } else if (currentMode == MODE_METER) {
      drawMeterMode();
    }
  }
}

// Global variable for scroll row
int scrollRow = 0;

void drawScrollMode() {
  // Manual scrolling by redrawing everything or just shifting the view.
  // SSD1306 doesn't have an easy "scroll display up" in the library that preserves content buffer easily.
  // We'll redraw based on the history of pulses.
  display.clearDisplay();

  // Plot each recent pulse as a horizontal dot.
  // We'll show the last 32 batches of pulses (one batch per horizontal line).
  // Actually, let's keep it simple: draw a distribution of current pulses on the bottom line
  // and manually move pixels up. Since clearDisplay() is fast, let's just draw the last few batches.

  // Better: draw a histogram-like distribution on the bottom part
  display.drawLine(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, SSD1306_WHITE);

  for (int i = 0; i < MAX_PULSES; i++) {
    if (pulses[i] >= MIN_PULSE && pulses[i] <= MAX_PULSE) {
      int x = map(pulses[i], MIN_PULSE, MAX_PULSE, 0, SCREEN_WIDTH - 1);
      display.drawPixel(x, SCREEN_HEIGHT - 1, SSD1306_WHITE);
    }
  }

  // Note: True scrolling like C64 "Head Tool" would require manual buffer manipulation.
  // To avoid `scrollDisplayUp` error, we just update the bottom line.
  // For better visual, we can use a small hack to simulate scrolling:
  // (In a real scenario, one might manipulate the display.getBuffer() directly)

  display.display();
}

void drawHeadFitMode() {
  display.clearDisplay();

  // Reference lines (Short, Medium, Long pulses for C64 ROM loader)
  int xS = map(352, MIN_PULSE, MAX_PULSE, 0, SCREEN_WIDTH-1);
  int xM = map(512, MIN_PULSE, MAX_PULSE, 0, SCREEN_WIDTH-1);
  int xL = map(672, MIN_PULSE, MAX_PULSE, 0, SCREEN_WIDTH-1);

  for(int y=0; y<5; y++) {
    display.drawPixel(xS, y, SSD1306_WHITE);
    display.drawPixel(xM, y, SSD1306_WHITE);
    display.drawPixel(xL, y, SSD1306_WHITE);
  }

  // Draw a distribution of pulses over the last history
  // Y axis represents "time" or "batch"
  for (int i = 0; i < 24; i++) {
    int idx = (pulseIndex - 1 - i + MAX_PULSES) % MAX_PULSES;
    if (pulses[idx] >= MIN_PULSE && pulses[idx] <= MAX_PULSE) {
      int x = map(pulses[idx], MIN_PULSE, MAX_PULSE, 0, SCREEN_WIDTH - 1);
      display.drawPixel(x, 8 + i, SSD1306_WHITE);
    }
  }
  display.display();
}

void drawMeterMode() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(F("Azimuth Quality"));

  int count = 0;
  for (int i = 0; i < MAX_PULSES; i++) {
    if (pulses[i] >= MIN_PULSE && pulses[i] <= MAX_PULSE) count++;
  }

  // Bar 1: Density
  int barWidth1 = map(count, 0, MAX_PULSES, 0, SCREEN_WIDTH);
  display.fillRect(0, 12, barWidth1, 6, SSD1306_WHITE);

  // Bar 2: Jitter
  unsigned long pMin = 1000, pMax = 0;
  int pCount = 0;
  for (int i = 0; i < 15; i++) {
    int idx = (pulseIndex - 1 - i + MAX_PULSES) % MAX_PULSES;
    if (pulses[idx] >= MIN_PULSE && pulses[idx] <= MAX_PULSE) {
        if (pulses[idx] < pMin) pMin = pulses[idx];
        if (pulses[idx] > pMax) pMax = pulses[idx];
        pCount++;
    }
  }
  if (pCount > 5) {
      int jitter = pMax - pMin;
      int barWidth2 = map(jitter, 0, 150, SCREEN_WIDTH, 0);
      if (barWidth2 < 0) barWidth2 = 0;
      if (barWidth2 > SCREEN_WIDTH) barWidth2 = SCREEN_WIDTH;
      display.fillRect(0, 22, barWidth2, 6, SSD1306_WHITE);
  }

  display.display();
}
