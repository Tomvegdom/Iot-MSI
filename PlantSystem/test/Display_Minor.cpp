#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  display.display();
}

void loop() {
  // Clear the display before each frame
  display.clearDisplay();

  // Display "MSI IoT" in the middle of the screen
  display.setTextSize(2);              // Set text size
  display.setTextColor(SSD1306_WHITE); // Set text color
  display.setCursor(25, 40);           // Set text position
  display.println(F("MSI IoT"));       // Display the text

  // Display the frame on the OLED
  display.display();

  // Delay to control the fps
  delay(300);
}