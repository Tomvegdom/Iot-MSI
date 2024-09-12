// Include libraries
#include <Arduino.h>
#include "DHT.h"
#include "switchKaKu.h"
#include <WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <math.h>

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

#ifdef __cplusplus
extern "C"
{
#endif
    uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

const char *DEVICE = "ESP_Tom";
const char *DEVICE_LOCATION = "Renswoude";

// Wi-Fi and Influx settings
WiFiMulti wifiMulti;

const char *WIFI_SSID = "iPhone van Tom";
const char *WIFI_PASSWORD = "TomTom12!";
const char *INFLUXDB_URL = "https://influx.mvrautomatisering.nl";
const char *INFLUXDB_TOKEN = "aMsyKoeLvJIU_vCODCYt1FEFmOibFZO9mdpZogxi223t9r_3afKUV4H6ZUm5cRS-H-IR7Nv5zFa80yC75ux-mw==";
const char *INFLUXDB_ORG = "f2f7bc17e4bedc06";
const char *INFLUXDB_BUCKET = "TomvanEgdom";

// Set timezone so the data will actually have a correct date and time
const char *TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3";

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Point object called deviceStatus with measurment name "devices"
Point deviceStatus("devices");

void setup()
{
    // Serial 9600 baudrate
    Serial.begin(9600);

    // Set the pins to output mode
    pinMode(pumpPin, OUTPUT);
    pinMode(ldrPin, INPUT);
    pinMode(ledPin, OUTPUT);
    lightInitial = analogRead(ldrPin);

    dht.begin();

    // Setup wifi
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

    // Sets the options for the Influx client. It will send data in batches of size 1 and it will send the data immediately (flushInterval 0)
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

    if (moistValue < 2000 && h < 55)
    {
        Serial.println("Pump is now on");
        digitalWrite(pumpPin, HIGH);
    }

    else
    {
        Serial.println("Pump is now off");
        digitalWrite(pumpPin, LOW);
    }
    // Clear fields for reusing the point. Tags will remain untouched
    deviceStatus.clearFields();

    // Store measured value into point
    // Report RSSI of currently connected network
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

    // Write light value
    unsigned long lightValue = millis();
    deviceStatus.addField("light", lightValue);

    // Write moisture value
    unsigned long moistValue = millis();
    deviceStatus.addField("moist", moistValue);

    // client.writePoint(deviceStatus);

    // Check Wi-Fi connection and reconnect if needed
    if (wifiMulti.run() != WL_CONNECTED)
    {
        Serial.println("Wifi connection lost");
    }

    // Write point
    if (!client.writePoint(deviceStatus))
    {
        Serial.print("InfluxDB write failed: ");
        Serial.println(client.getLastErrorMessage());
    }

    // Delay 500 ms
    Serial.println("Wait 0.5s");
    delay(500);
}