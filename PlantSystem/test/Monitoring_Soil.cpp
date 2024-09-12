#include <Arduino.h>

//Set pin number
int moistSensPin = 36;
int moistValue = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
//Save the moisture value to a variable
moistValue = analogRead(moistSensPin);

//Print the value to the serial output
Serial.println(moistValue);

//Wait for one second
delay(1000);
}