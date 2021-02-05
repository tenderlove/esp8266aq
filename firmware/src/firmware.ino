#include <SI7021.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WiFiMulti.h>
#include <Wire.h>
#include <base64.h>
#include <WiFiManager.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

ESP8266WiFiMulti WiFiMulti;
WiFiUDP udp;
IPAddress broadcastIp(224, 0, 0, 1);
Adafruit_BME280 bme;

byte inputString[32];
int i = 0;
int recordId = 0;

#define LED_PIN 2
#define G2_PIN 14
#define G3_PIN 12

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());

  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup() {
  WiFiManager wifiManager;

  pinMode(G2_PIN, INPUT);
  pinMode(G3_PIN, INPUT);

  Serial.begin(9600);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.autoConnect();

  delay(500);

  if (!bme.begin(0x76)) {
    Serial.println("Couldn't find BME280 sensor");
    delay(50);
  }
  Serial.swap(); // Switch to sensor
}

#define WRITE_INFO_TO(output) \
  output.print("{\"mac\":\""); \
  output.print(WiFi.macAddress()); \
  output.print("\",\"version\":1,\"record_id\":"); \
  output.print(recordId); \
  output.print(",\"aq\":\""); \
  output.print(encoded); \
  output.print("\",\"temperature\":"); \
  output.print(temperature); \
  output.print(",\"humidity\":"); \
  output.print(humidity); \
  output.print("}");

void printToSerial(int temperature, int humidity, String encoded) {
  Serial.swap(); // Switch to main serial
  WRITE_INFO_TO(Serial)
  Serial.flush();
  Serial.swap(); // Switch to sensor
}

void printToUdp(int temperature, int humidity, String encoded) {
  udp.beginPacketMulticast(broadcastIp, 8000, WiFi.localIP());
  WRITE_INFO_TO(udp)
  udp.endPacket();
}

void loop() {
  while (Serial.available()) {
    inputString[i] = Serial.read();
    i++;
    if (i == 1 && inputString[0] != 0x42) {
      i = 0;
    } else if (i == 2 && inputString[1] != 0x4d) {
      i = 0;
    } else if (i == 32) {
      i = 0;

      int temperature = round(bme.readTemperature() * 100);
      int humidity = round(bme.readHumidity() * 100);

      String encoded = base64::encode(inputString, 32);

      printToSerial(temperature, humidity, encoded);
      printToUdp(temperature, humidity, encoded);
      recordId++;
    }
  }
}
