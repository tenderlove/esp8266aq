#include <SI7021.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WiFiMulti.h>
#include <Wire.h>
#include <base64.h>
#include <WiFiManager.h>

#define SDA 0 // GPIO0 on ESP-01 module
#define SCL 2 // GPIO2 on ESP-01 module

ESP8266WiFiMulti WiFiMulti;
WiFiUDP udp;
IPAddress broadcastIp(224, 0, 0, 1);
SI7021 sensor;

byte inputString[32];
int i = 0;
int recordId = 0;

void setup() {
  WiFiManager wifiManager;

  wifiManager.autoConnect();

  delay(500);

  Serial.begin(9600);
  Wire.begin(0, 2);
  sensor.begin(SDA, SCL);
}

void loop() {
  while (Serial.available()) {
    inputString[i] = Serial.read();
    i++;
    if (i == 2) { // Check for start of packet
      if (!(inputString[0] == 0x42 && inputString[1] == 0x4d)) {
        i = 0;
      }
    }
    if (i == 32) {
      i = 0;

      auto weather = sensor.getHumidityAndTemperature();
      int temperature = weather.celsiusHundredths;
      int humidity = weather.humidityBasisPoints;

      String encoded = base64::encode(inputString, 32);
      udp.beginPacketMulticast(broadcastIp, 9000, WiFi.localIP());
      udp.print("{\"mac\":\"");
      udp.print(WiFi.macAddress());
      udp.print("\",\"record_id\":");
      udp.print(recordId);
      udp.print(",\"aq\":\"");
      udp.print(encoded);
      udp.print("\",\"temperature\":");
      udp.print(temperature);
      udp.print(",\"humidity\":");
      udp.print(humidity);
      udp.print("}");
      udp.endPacket();
      recordId++;
    }
  }
}
