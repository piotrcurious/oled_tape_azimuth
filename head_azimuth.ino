
// Include the Adafruit OLED library
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define the OLED display size and address
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_ADDR   0x3C // OLED display address

// Create an Adafruit OLED object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pins
#define TAPE_PIN 3      // Pulse input
#define BUTTON_PIN 2    // Mode switch button

// Constants for C64 pulse lengths (in microseconds)
#define C64_SHORT 284
#define C64_MEDIUM 396
#define C64_LONG 508

#define MIN_PULSE 150
#define MAX_PULSE 700
#define MAX_PULSES 128

// Pulse history
unsigned long pulses[MAX_PULSES];
int pulseIndex = 0;
unsigned long lastUpdateTime = 0;
#define DISPLAY_INTERVAL 40

// Mode definitions
enum Mode {
  MODE_SCROLL,    // Scroll distribution
  MODE_HEADFIT,   // C64 "Head Fit" style (scatter)
  MODE_METER,     // Quality bar meters
  MODE_STATS      // Numeric jitter/quality stats
};

Mode currentMode = MODE_SCROLL;
bool lastButtonState = HIGH;

// Prototypes
void drawScrollMode();
void drawHeadFitMode();
void drawMeterMode();
void drawStatsMode();

void setup() {
  Serial.begin(9600);
  pinMode(TAPE_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Splash screen (C64 Style)
  display.setCursor(20, 8);
  display.println(F("**** COMMODORE 64 ****"));
  display.setCursor(15, 18);
  display.println(F("64K RAM SYSTEM  38911"));
  display.display();
  delay(1500);

  display.clearDisplay();
  display.setCursor(10, 10);
  display.println(F("AZIMUTH TOOL v3.0"));
  display.display();
  delay(1000);
}

void loop() {
  // Mode switching
  bool buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH) {
    currentMode = (Mode)((currentMode + 1) % 4);
    display.clearDisplay();
    display.setCursor(0,0);
    display.print(F("Mode: "));
    switch(currentMode) {
      case MODE_SCROLL:  display.println(F("HISTOGRAM")); break;
      case MODE_HEADFIT: display.println(F("HEAD FIT")); break;
      case MODE_METER:   display.println(F("METER")); break;
      case MODE_STATS:   display.println(F("STATS")); break;
    }
    display.display();
    delay(400);
    lastUpdateTime = millis();
  }
  lastButtonState = buttonState;

  // Pulse measurement
  unsigned long pulseLength = pulseIn(TAPE_PIN, HIGH, 15000);
  if (pulseLength > 0) {
    pulses[pulseIndex] = pulseLength;
    pulseIndex = (pulseIndex + 1) % MAX_PULSES;
  }

  // UI update
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime > DISPLAY_INTERVAL) {
    lastUpdateTime = currentTime;
    switch(currentMode) {
      case MODE_SCROLL:  drawScrollMode(); break;
      case MODE_HEADFIT: drawHeadFitMode(); break;
      case MODE_METER:   drawMeterMode(); break;
      case MODE_STATS:   drawStatsMode(); break;
    }
  }
}

void drawScrollMode() {
  display.clearDisplay();
  display.drawLine(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, SSD1306_WHITE);

  int xS = map(C64_SHORT, MIN_PULSE, MAX_PULSE, 0, SCREEN_WIDTH - 1);
  int xM = map(C64_MEDIUM, MIN_PULSE, MAX_PULSE, 0, SCREEN_WIDTH - 1);
  int xL = map(C64_LONG, MIN_PULSE, MAX_PULSE, 0, SCREEN_WIDTH - 1);
  display.drawPixel(xS, SCREEN_HEIGHT - 2, SSD1306_WHITE);
  display.drawPixel(xM, SCREEN_HEIGHT - 2, SSD1306_WHITE);
  display.drawPixel(xL, SCREEN_HEIGHT - 2, SSD1306_WHITE);

  uint8_t bins[SCREEN_WIDTH];
  memset(bins, 0, SCREEN_WIDTH);
  for (int i = 0; i < MAX_PULSES; i++) {
    if (pulses[i] >= MIN_PULSE && pulses[i] <= MAX_PULSE) {
      int x = map(pulses[i], MIN_PULSE, MAX_PULSE, 0, SCREEN_WIDTH - 1);
      if (bins[x] < SCREEN_HEIGHT - 4) bins[x]++;
    }
  }

  for (int x = 0; x < SCREEN_WIDTH; x++) {
    if (bins[x] > 0) {
      display.drawLine(x, SCREEN_HEIGHT - 2, x, SCREEN_HEIGHT - 2 - bins[x], SSD1306_WHITE);
    }
  }
  display.display();
}

void drawHeadFitMode() {
  display.clearDisplay();

  int xS = map(C64_SHORT, MIN_PULSE, MAX_PULSE, 0, SCREEN_WIDTH - 1);
  int xM = map(C64_MEDIUM, MIN_PULSE, MAX_PULSE, 0, SCREEN_WIDTH - 1);
  int xL = map(C64_LONG, MIN_PULSE, MAX_PULSE, 0, SCREEN_WIDTH - 1);

  display.drawLine(xS, 0, xS, 2, SSD1306_WHITE);
  display.drawLine(xS, SCREEN_HEIGHT - 3, xS, SCREEN_HEIGHT - 1, SSD1306_WHITE);
  display.drawLine(xM, 0, xM, 2, SSD1306_WHITE);
  display.drawLine(xM, SCREEN_HEIGHT - 3, xM, SCREEN_HEIGHT - 1, SSD1306_WHITE);
  display.drawLine(xL, 0, xL, 2, SSD1306_WHITE);
  display.drawLine(xL, SCREEN_HEIGHT - 3, xL, SCREEN_HEIGHT - 1, SSD1306_WHITE);

  for (int i = 0; i < SCREEN_HEIGHT - 6; i++) {
    int idx = (pulseIndex - 1 - i + MAX_PULSES) % MAX_PULSES;
    if (pulses[idx] >= MIN_PULSE && pulses[idx] <= MAX_PULSE) {
      int x = map(pulses[idx], MIN_PULSE, MAX_PULSE, 0, SCREEN_WIDTH - 1);
      display.drawPixel(x, 3 + i, SSD1306_WHITE);
    }
  }
  display.display();
}

void drawMeterMode() {
  display.clearDisplay();

  int count = 0;
  for (int i = 0; i < MAX_PULSES; i++) {
    if (pulses[i] >= MIN_PULSE && pulses[i] <= MAX_PULSE) count++;
  }

  display.setCursor(0,0);
  display.print(F("SIGNAL: "));
  display.fillRect(45, 0, map(count, 0, MAX_PULSES, 0, SCREEN_WIDTH - 45), 7, SSD1306_WHITE);

  unsigned long pMin = 1000, pMax = 0;
  int pCount = 0;
  for (int i = 0; i < 16; i++) {
    int idx = (pulseIndex - 1 - i + MAX_PULSES) % MAX_PULSES;
    if (pulses[idx] >= MIN_PULSE && pulses[idx] <= MAX_PULSE) {
        if (pulses[idx] < pMin) pMin = pulses[idx];
        if (pulses[idx] > pMax) pMax = pulses[idx];
        pCount++;
    }
  }

  display.setCursor(0, 12);
  display.print(F("STABLE: "));
  if (pCount > 5) {
      int jitter = pMax - pMin;
      int quality = 100 - (jitter / 2);
      if (quality < 0) quality = 0;
      display.fillRect(45, 12, map(quality, 0, 100, 0, SCREEN_WIDTH - 45), 7, SSD1306_WHITE);
  }

  display.display();
}

void drawStatsMode() {
  display.clearDisplay();

  unsigned long pMin = 1000, pMax = 0, pAvg = 0;
  int pCount = 0;
  for (int i = 0; i < 32; i++) {
    int idx = (pulseIndex - 1 - i + MAX_PULSES) % MAX_PULSES;
    if (pulses[idx] >= MIN_PULSE && pulses[idx] <= MAX_PULSE) {
        if (pulses[idx] < pMin) pMin = pulses[idx];
        if (pulses[idx] > pMax) pMax = pulses[idx];
        pAvg += pulses[idx];
        pCount++;
    }
  }
  if (pCount > 0) pAvg /= pCount;

  display.setCursor(0, 0);
  display.print(F("AVG: ")); display.print(pAvg);
  display.setCursor(SCREEN_WIDTH/2, 0);
  display.print(F("CNT: ")); display.print(pCount);
  display.setCursor(0, 10);
  display.print(F("MIN: ")); display.print(pMin);
  display.setCursor(SCREEN_WIDTH/2, 10);
  display.print(F("MAX: ")); display.print(pMax);
  display.setCursor(0, 20);
  display.print(F("JITTER: ")); display.print(pMax - pMin);

  display.display();
}
