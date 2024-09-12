#include <Arduino.h>

const int ldrPin = 39;
const int ledPin = 2;

int lightInitial;
int lightValue;

void setup() {
  Serial.begin(9600);
  pinMode(ldrPin, INPUT);
  pinMode(ledPin, OUTPUT);
  lightInitial = analogRead(ldrPin);
}

void loop() {
  lightValue = analogRead(ldrPin);
  Serial.println(lightValue);
  if(lightInitial - lightValue >= 200) {
  digitalWrite (ledPin, HIGH); // turn on light
}

  else {
    digitalWrite (ledPin, LOW); // turn off light
  }

}