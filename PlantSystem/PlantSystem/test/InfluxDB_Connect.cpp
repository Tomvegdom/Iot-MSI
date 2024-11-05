#include <Arduino.h>
#include <WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <math.h>

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

// Set timezone so your data will actually have a correct date and time
const char *TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3";

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Point object called deviceStatus with measurment name "devices"
Point deviceStatus("devices");

void setup()
{
    // Serial is always good to use for debugging!
    Serial.begin(9600);

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

    // This line sets the options for the Influx client. It means that it will send data in batches of size 1 and it will send the data immediately (flushInterval 0)
    client.setWriteOptions(WriteOptions().batchSize(1).flushInterval(0));

    // Here you can add tags as you please.
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

    Serial.println("Wait 10s");
    delay(10000);
}