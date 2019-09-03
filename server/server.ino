#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WiFiMulti.h>
#include <base64.h>

#ifndef STASSID
#define STASSID "WifiAPName"
#define STAPSK  "WifiPassword"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

ESP8266WiFiMulti WiFiMulti;
WiFiUDP udp;
IPAddress broadcastIp(224, 0, 0, 1);

byte inputString[32];
int i = 0;
int recordId = 0;

void setup() {
  Serial.begin(9600);

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
  }

  delay(500);
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
      String encoded = base64::encode(inputString, 32);
      udp.beginPacketMulticast(broadcastIp, 9000, WiFi.localIP());
      udp.print("[\"aq\",{\"mac\":\"");
      udp.print(WiFi.macAddress());
      udp.print("\",\"record_id\":");
      udp.print(recordId);
      udp.print(",\"record\":\"");
      udp.print(encoded);
      udp.print("\"}]");
      udp.endPacket();
      recordId++;
    }
  }
}
