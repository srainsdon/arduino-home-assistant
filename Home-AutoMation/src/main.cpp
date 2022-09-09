#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include <ArduinoHA.h>
#include "DHT.h"

// DHT Settings
#define DHTPIN D1     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22
DHT dht(DHTPIN, DHTTYPE);
float t;
float h;
#define STATUS_LED 16

//Heater Connected Here 
#define HEATPIN D2     // Digital pin connected to the Hearter Realy

// Home Assistant Settings
#define HA_DEVICE_NAME    "MainRoom.Thermometer"
#define SOFTWARE_VERSION  "0.0.1"

// WIFI SETTINGS
#define WIFI_SSID         "FBI-024"
#define WIFI_PASSWORD     "Weapon Push 6"
WiFiClient client;

//MQTT Settings
#define BROKER_ADDR IPAddress(10,15,0,25)
#define BROKER_USER "Mqtt"
#define BROKER_PASS "Mqtt"

HADevice device;
HAMqtt mqtt(client, device);
HASensor temp("rtemp");
HASensor humid("rhumid"); 

//Other settings
unsigned long lastSentAt = millis();
unsigned long lastBlinkAt = millis();


void setup() {
    pinMode(STATUS_LED, OUTPUT);
    Serial.begin(9600);
    Serial.println("Starting...");
    byte mac[WL_MAC_ADDR_LENGTH];
    WiFi.macAddress(mac);
    device.setUniqueId(mac, sizeof(mac));

     // connect to wifi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500); // waiting for the connection
    }
    Serial.println();
    Serial.println("Connected to the network");
    digitalWrite(STATUS_LED, HIGH);
     // set device's details (optional)
    device.setName(HA_DEVICE_NAME);
    device.setSoftwareVersion(SOFTWARE_VERSION);

    // configure sensor (optional)
    temp.setUnitOfMeasurement("°C");
    temp.setDeviceClass("temperature");
    temp.setIcon("mdi:thermometer");
    temp.setName("Temperature");

    // configure sensor (optional)
    humid.setUnitOfMeasurement("%");
    humid.setDeviceClass("humidity");
    //humid.setIcon("mdi:home");
    humid.setName("Humidity");

    mqtt.begin(BROKER_ADDR, BROKER_USER, BROKER_PASS);
    dht.begin();
    Serial.println(F("Setup Finished"));
}

void loop() {
    
    mqtt.loop();

    if ((millis() - lastBlinkAt) >= 45000) {
        lastBlinkAt = millis();
        digitalWrite(STATUS_LED, LOW);
    }

    if ((millis() - lastBlinkAt) >= 1000) {
      digitalWrite(STATUS_LED, HIGH);
    }
    
    

    if ((millis() - lastSentAt) >= 5000) {
        lastSentAt = millis();
        t = dht.readTemperature();
        h = dht.readHumidity();
        if (isnan(h) || isnan(t)) {
          Serial.println(F("Failed to read from DHT sensor!"));
          return;
        }
        Serial.print(F("Reporting Room as "));
        Serial.print(t);
        Serial.print(F("°C With a Humidity of "));
        Serial.print(h);
        Serial.println(F("%"));
        temp.setValue(t);
        humid.setValue(h);
    }
}
