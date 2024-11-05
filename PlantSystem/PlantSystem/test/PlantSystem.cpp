// Include libraries
#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#include "switchKaKu.h"

#define DHTPIN 32
#define DHTTYPE DHT11
#define TRANSMITTERID1 046241
#define rfPin 25
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Set sensor pin numbers
const int ledPin = 2;
const int ldrPin = 39;
const int pumpPin = 33;
const int moistSensPin = 36;
int moistValue;
int lightInitial;
int lightValue;
float h;
float t;

// Wi-Fi and Influx settings
WiFiMulti wifiMulti;
const char *WIFI_SSID = "iPhone van Tom";
const char *WIFI_PASSWORD = "TomTom12!";
const char *INFLUXDB_URL = "https://influx.mvrautomatisering.nl";
const char *INFLUXDB_TOKEN = "Jef24nCItLll61tZhtargUTASvCIrQcAsb-oZpWzrxqXcBl9COkRYns5-mabjc1V3LaMpippL1vpaUefR8Y7lw==";
const char *INFLUXDB_ORG = "f2f7bc17e4bedc06";
const char *INFLUXDB_BUCKET = "TomvanEgdom";
const char *DEVICE = "ESP_Tom";
const char *DEVICE_LOCATION = "Renswoude";
const char *TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3";

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Point object called deviceStatus with measurment name "devices"
Point deviceStatus("devices");

// Setup DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Setup screen parameters
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Read internal temperature
#ifdef __cplusplus
extern "C"
{
#endif
  uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

void setup()
{
  // Start serial 9600 baudrate
  Serial.begin(9600);

  // Set the pins to the desired mode
  pinMode(ledPin, OUTPUT);
  pinMode(ldrPin, INPUT);
  pinMode(pumpPin, OUTPUT);
  pinMode(moistSensPin, INPUT);
  lightInitial = analogRead(ldrPin);
  dht.begin();

  // Setup wifi connection
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi...");
  while (wifiMulti.run() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Sets the options for the Influx client. It will send data in batches of size 1 and it will send the data immediately
  client.setWriteOptions(WriteOptions().batchSize(1).flushInterval(0));

  // Tag names
  deviceStatus.addTag("device", DEVICE);
  deviceStatus.addTag("location", DEVICE_LOCATION);
  deviceStatus.addTag("SSID", WiFi.SSID());

  // Sync time for proper connection and data points
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection())
  {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  }
  else
  {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  display.clearDisplay();
  display.display();
}

void sensors()
{
  // Save the light value to a variable
  lightValue = analogRead(ldrPin);

  // Save the moisture value to a variable
  moistValue = analogRead(moistSensPin);

  // Reading temperature or humidity takes about 250 milliseconds!
  h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t))
  {
    Serial.println(F("Failed to read DHT sensor!"));
    return;
  }
}

void oled()
{
  // Clear the display before each frame
  display.clearDisplay();

  // Display "MSI IoT" in the middle of the screen
  display.setTextSize(2);              // Set text size
  display.setTextColor(SSD1306_WHITE); // Set text color
  display.setCursor(25, 5);            // Set text position
  display.println(F("MSI IoT"));       // Display the text
  display.setTextSize(1);              // Set text size
  display.setCursor(5, 25);            // Set text position
  display.print("Light value: ");      // Display the text
  display.println(lightValue);         // Display the text
  display.setCursor(5, 35);            // Set text position
  display.print("Moist value: ");      // Display the text
  display.println(moistValue);         // Display the text
  display.setCursor(5, 45);            // Set text position
  display.print("Humidity: ");         // Display the text
  display.print(h);                    // Display the text
  display.println("%");                // Display the text
  display.setCursor(5, 55);            // Set text position
  display.print("Temperature: ");      // Display the text
  display.print(t);                    // Display the text
  display.println("C");                // Display the text

  // Display the frame on the OLED
  display.display();

  // Clear fields for reusing the point. Tags will remain untouched
  deviceStatus.clearFields();
}

void loop()
{
  // Run sensors and oled loop
  sensors();
  oled();

  if (lightInitial - lightValue >= 200)
  {
    Serial.println("Kaku is on");
    switchKaku(rfPin, TRANSMITTERID1, 1, 1, true, 3); // Turn on Kaku
  }

  else
  {
    Serial.println("Kaku is off");
    switchKaku(rfPin, TRANSMITTERID1, 1, 1, false, 3); // Turn off KaKu
  }

  while (moistValue < 2500)
  {

    Serial.println("Pump is on");
    digitalWrite(pumpPin, HIGH); // Turn on pump
    delay(5000);
    sensors();
    oled();
  }

  Serial.println("Pump is off"); // Turn off pump
  digitalWrite(pumpPin, LOW);

  // Write light value
  deviceStatus.addField("light_value", lightValue);

  // Write moist value
  deviceStatus.addField("moist_value", moistValue);

  // Write humidity value
  deviceStatus.addField("humidity", h);

  // Write temperature value
  deviceStatus.addField("temperature", t);

  // Write internal temperature
  int t_f = temprature_sens_read();
  int t_c = (t_f - 32) * 5 / 9;
  deviceStatus.addField("internal_temp", t_c);

  // Write WiFi strength
  int wifiStrength = WiFi.RSSI();
  deviceStatus.addField("wifi_strength", wifiStrength);

  // Write uptime
  unsigned long uptime = millis() / 1000;
  deviceStatus.addField("uptime", uptime);

  // Check Wi-Fi connection and reconnect if needed
  if (wifiMulti.run() != WL_CONNECTED)
  {
    Serial.println("Wifi connection lost");
  }

  // Turn on led when writing starts
  digitalWrite(ledPin, HIGH);

  // Write point
  if (!client.writePoint(deviceStatus))
  {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  // Turn off led when writing is done
  digitalWrite(ledPin, LOW);

  // Wait 5 seconds
  delay(5000);
}