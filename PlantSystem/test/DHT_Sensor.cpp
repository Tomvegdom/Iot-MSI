#include <Arduino.h>
#include "DHT.h"

#define DHTPIN 32
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
int pumpPin = 33;

//Set pin number
int moistSensPin = 36;
int moistValue = 0;

void setup() {
  Serial.begin(9600);
  Serial.println(F("DHT11 test!"));

  dht.begin();

  //set the pin to output mode
pinMode(pumpPin, OUTPUT);
}

void loop() {
//Save the moisture value to a variable
moistValue = analogRead(moistSensPin);

//Print the value to the serial output
Serial.println(moistValue);

 // Wait a few seconds between measurements.
  delay(1000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) ){
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));
  
//Wait for one second
delay(1000);

 Serial.println("Pump is now on");
 digitalWrite(pumpPin, HIGH);
delay(500);

Serial.println("Pump is now off");
digitalWrite(pumpPin, LOW);
delay(500);
}