/* wifi setup by 9SQ https://github.com/9SQ/esp8266-wifi-setup */

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "ThingSpeak.h"

const char* ssid = "SSID"; // Wifi ssid
const char* password = "PASSWORD"; // Wifi password

WiFiClient  client;

const IPAddress apIP(192, 168, 1, 1);
const char* apSSID = "ESP8266_SETUP";
boolean settingMode;
String ssidList;

DNSServer dnsServer;
ESP8266WebServer webServer(80);

unsigned long myChannelNumber; //thingspeak channel number
const char * myWriteAPIKey; //write key for thingspeak channel

#define DHTPIN     2         // Pin connected to the DHT sensor.
#define DHTTYPE    DHT11     // DHT 11
DHT_Unified dht(DHTPIN, DHTTYPE);

float avgt = 0;
float avgh = 0;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  delay(10);
  if (restoreConfig()) {
      if (checkConnection()) {
          settingMode = false;
          startWebServer();
          return;
      }
  }
  settingMode = true;
  setupMode();

  dht.begin();
  sensor_t sensor;
}

void loop() {
    if (settingMode) {
        dnsServer.processNextRequest();
    }

    webServer.handleClient();
  for (int x = 0; x < 15; x++) {
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    avgt = avgt + event.temperature;
    dht.humidity().getEvent(&event);
    avgh = avgh + event.relative_humidity;
    delay(20000);
  }
  avgt = avgt / 16;
  avgh = avgh / 16;
  Serial.print("Temperature: ");
  Serial.print(avgt);
  Serial.print("  |  Humidity: ");
  Serial.println(avgh);

  ThingSpeak.setField(1, avgt);
  ThingSpeak.setField(2, avgh);
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
}