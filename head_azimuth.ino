
// Include the Adafruit OLED library
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define the OLED display size and address
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_ADDR   0x3C // OLED display address

// Create an Adafruit OLED object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Define the analog input pin for the tape signal
#define TAPE_PIN A0

// Define the threshold for detecting a pulse
#define PULSE_THRESHOLD 512

// Define the maximum number of pulses to store
#define MAX_PULSES 128

// Define the time interval for scrolling the display (in milliseconds)
#define SCROLL_INTERVAL 100

// Create an array to store the pulse lengths
unsigned long pulses[MAX_PULSES];

// Create a variable to store the current pulse index
int pulseIndex = 0;

// Create a variable to store the last scroll time
unsigned long lastScrollTime = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

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
  display.println(F("FSK Pulse Length Plotter"));
  display.display();
}

void loop() {
  // Read the analog value from the tape pin
  int analogValue = analogRead(TAPE_PIN);

  // Check if the value is above the threshold
  if (analogValue > PULSE_THRESHOLD) {
    // Start measuring the pulse length
    unsigned long pulseLength = pulseIn(TAPE_PIN, HIGH);

    // Store the pulse length in the array
    pulses[pulseIndex] = pulseLength;

    // Increment the pulse index and wrap around if necessary
    pulseIndex = (pulseIndex + 1) % MAX_PULSES;

    // Print the pulse length to serial monitor
    Serial.println(pulseLength);
  }

  // Get the current time
  unsigned long currentTime = millis();

  // Check if it is time to scroll the display
  if (currentTime - lastScrollTime > SCROLL_INTERVAL) {
    // Update the last scroll time
    lastScrollTime = currentTime;

    // Scroll the display one line up
    display.scrollDisplayUp();

    // Draw a horizontal line at the bottom of the display
    display.drawLine(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, SSD1306_WHITE);

    // Plot the pulse lengths on the bottom line of the display
    for (int i = 0; i < MAX_PULSES; i++) {
      // Map the pulse length to a pixel position on the x-axis
      int x = map(i, 0, MAX_PULSES - 1, 0, SCREEN_WIDTH - 1);

      // Map the pulse length to a pixel position on the y-axis
      int y = map(pulses[i], 0, SCROLL_INTERVAL * MAX_PULSES / 10, SCREEN_HEIGHT - 2, 0);

      // Draw a pixel at the corresponding position
      display.drawPixel(x, y, SSD1306_WHITE);
    }

    // Update the display
    display.display();
  }
}

//Source: Conversation with Bing, 4/27/2023
//(1) pulseIn() - Arduino Reference. https://reference.arduino.cc/reference/cs/language/functions/advanced-io/pulsein/.
//(2) Arduino - Home. https://www.arduino.cc/reference/en/language/functions/advanced-io/pulsein/.
//(3) Basics of PWM (Pulse Width Modulation) | Arduino Documentation. https://www.arduino.cc/en/Tutorial/PWM.
