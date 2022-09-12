
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
// #include <ESP8266WebServer.h>

#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h> // Library for LCD

#include <ArduinoHA.h>
#include "DHT.h"

#include <NTPClient.h>

// DHT Settings
#define DHTPIN D3     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22
DHT dht(DHTPIN, DHTTYPE);
float t;
float h;

// Home Assistant Settings
#define HA_DEVICE_NAME    "TMP.MainRoom.Thermometer"
#define SOFTWARE_VERSION  "0.1.5" 

// 0.0.1 was getting it working...
// 0.1.* will be getting LCD working with it.
// 0.1.1 learned how to control the back light
// 0.1.3 Got NTP working
// 0.1.4 testing time offset

// WIFI SETTINGS
#define WIFI_SSID         "FBI-024"
#define WIFI_PASSWORD     "Weapon Push 6"
WiFiClient client;

//MQTT Settings
#define BROKER_ADDR IPAddress(10,15,0,25)
#define BROKER_USER "Mqtt"
#define BROKER_PASS "Mqtt"

#define BLINKLED false
#define STATUS_LED 16


HADevice device;
HAMqtt mqtt(client, device);
HASensor temp("rtemp");
HASensor humid("rhumid"); 

//Other settings
unsigned long lastSentAt = millis();
unsigned long lastBlinkAt = millis();
unsigned long lastTimeAt = millis();

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 20, 4);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, IPAddress(192,168,1,1));

void timeTick () {
    if ((millis() - lastTimeAt) >= 1000) {
        lastTimeAt = millis();
        Serial.print(timeClient.getDay());
        Serial.print(" ");
        Serial.println(timeClient.getFormattedTime());
        lcd.setCursor(0, 0);
        lcd.print(timeClient.getFormattedTime());
    }
}

void setup() {
    pinMode(STATUS_LED, OUTPUT);
    Serial.begin(9600);
    Serial.println("Starting...");
    byte mac[WL_MAC_ADDR_LENGTH];
    WiFi.macAddress(mac);
    device.setUniqueId(mac, sizeof(mac));

    lcd.init();
    lcd.backlight();
    lcd.setCursor(1, 0); 
    lcd.print("Starting..."); 
    lcd.setCursor(1, 1); 
    lcd.print("EnvMon ");
    lcd.print(SOFTWARE_VERSION); 
    lcd.setCursor(1, 2); 
    lcd.print("By: Seth Rainsdon");

     // connect to wifi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500); // waiting for the connection
    }

    timeClient.begin();
    timeClient.setTimeOffset(-25200);
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
    delay(500);
    lcd.clear();
}

void loop() {

    mqtt.loop();
    timeClient.update();
    timeTick();

if (BLINKLED) {
    if ((millis() - lastBlinkAt) >= 45000) {
        lastBlinkAt = millis();
        digitalWrite(STATUS_LED, LOW);
    }

    if ((millis() - lastBlinkAt) >= 500) {
      digitalWrite(STATUS_LED, HIGH);
      
    }
}

if ((millis() - lastSentAt) >= 5000) {
        lastSentAt = millis();
        // lcd.backlight();
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
        // lcd.clear();
        lcd.setCursor(9, 0);
        lcd.print("Main Room");
        lcd.setCursor(0, 1);
        lcd.print("Temperature: ");
        float fahrenheit = (t * 9) / 5 + 32;
        lcd.print(fahrenheit);
        lcd.print("F");
        lcd.setCursor(0, 2);
        lcd.print("Rel Humidity: ");
        lcd.print(h);
        lcd.print("%");
        temp.setValue(t);
        humid.setValue(h);
    }
}