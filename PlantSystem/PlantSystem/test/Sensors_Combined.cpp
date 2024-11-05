// Include libraries
#include <Arduino.h>
#include "DHT.h"
#include "switchKaKu.h"

#define DHTPIN 32
#define DHTTYPE DHT11
#define TRANSMITTERID1 046241
#define rfPin 25

DHT dht(DHTPIN, DHTTYPE);
int pumpPin = 33;

// Set ldr pin number
const int ldrPin = 39;
const int ledPin = 2;

int lightInitial;
int lightValue;

// Set moist sensor pin number
int moistSensPin = 36;
int moistValue = 0;

void setup()
{
  Serial.begin(9600);
  // Set the pins to output mode
  pinMode(pumpPin, OUTPUT);
  pinMode(ldrPin, INPUT);
  pinMode(ledPin, OUTPUT);
  lightInitial = analogRead(ldrPin);

  dht.begin();
}

void loop()
{
  // Save the light value to a variable
  lightValue = analogRead(ldrPin);

  // Print the value to the serial output
  Serial.print(F("Light: "));
  Serial.println(lightValue);

  if (lightInitial - lightValue >= 200)
  {
    Serial.println("LED is now on");
    digitalWrite(ledPin, HIGH); // Turn on light
    Serial.println("Kaku is now on");
    switchKaku(rfPin, TRANSMITTERID1, 1, 1, true, 3); // Turn off Kaku
  }

  else
  {
    Serial.println("LED is now off");
    digitalWrite(ledPin, LOW); // Turn off light
    Serial.println("Kaku is now off");
    switchKaku(rfPin, TRANSMITTERID1, 1, 1, false, 3); // Turn off KaKu
  }

  // Save the moisture value to a variable
  moistValue = analogRead(moistSensPin);

  // Print the value to the serial output
  Serial.print(F("Moist: "));
  Serial.println(moistValue);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));

  if (moistValue < 2000 && h < 55)  {
    Serial.println("Pump is now on");
    digitalWrite(pumpPin, HIGH);
  }

  else
  {
    Serial.println("Pump is now off");
    digitalWrite(pumpPin, LOW);
  }
}