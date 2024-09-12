#include <Arduino.h>
int pumpPin = 33;

//Set pin number
int moistSensPin = 36;
int moistValue = 0;

void setup() {
  Serial.begin(9600);

  //set the pin to output mode
pinMode(pumpPin, OUTPUT);
}

void loop() {
//Save the moisture value to a variable
moistValue = analogRead(moistSensPin);

//Print the value to the serial output
Serial.println(moistValue);

//Wait for one second
delay(1000);

 Serial.println("Pump is now on");
 digitalWrite(pumpPin, HIGH);
delay(500);

Serial.println("Pump is now off");
digitalWrite(pumpPin, LOW);
delay(500);
}