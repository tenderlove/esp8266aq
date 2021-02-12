#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <PubSubClient.h>

Adafruit_BME280 bme;

WiFiClient espClient;
PubSubClient client(espClient);

const char *mqtt_server = "10.0.1.211";
const char *mqtt_prefix = "home/test/esp8266aq";

byte inputString[32];
int inputIdx = 0;

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

  randomSeed(micros());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  if (!bme.begin(0x76)) {
    Serial.println("Couldn't find BME280 sensor");
    delay(50);
  }

  Serial.flush();
  client.setServer(mqtt_server, 1883);
  Serial.swap(); // Switch to sensor
}

void mqtt_reconnect() {
  static unsigned long last_attempt = 0;
  if (millis() - last_attempt < 5000) {
    return;
  }

  String clientId = "ESP8266AQ-";
  clientId += String(random(0xffff), HEX);
  client.connect(clientId.c_str());
}

#define ARRAY_SIZE(array) ( sizeof( array )/sizeof( array[0] ) )

const char *pms5003_topics[] = {
  "pm1_0_standard",
  "pm2_5_standard",
  "pm10_standard",
  "pm1_0_env",
  "pm2_5_env",
  "concentration_unit",
  "particles_03um",
  "particles_05um",
  "particles_10um",
  "particles_25um",
  "particles_50um",
  "particles_100um"
};

void mqtt_publish(const char *topic, const char *format, ...) {
  char full_topic[256];
  char value[256];
  sprintf(full_topic, "%s/%s", mqtt_prefix, topic);

  va_list args;
  va_start(args, format);
  vsprintf(value, format, args);
  va_end(args);

  Serial.print(full_topic);
  Serial.print(" ");
  Serial.println(value);
  client.publish(full_topic, value, true);
}

void loop() {
  if (!client.connected()) {
    mqtt_reconnect();
  }

  if (!client.connected()) {
    Serial.swap();
    Serial.println("MQTT client is not connected");
    Serial.flush();
    Serial.swap();
  }
  client.loop();

  while (Serial.available()) {
    inputString[inputIdx] = Serial.read();
    inputIdx++;
    if (inputIdx == 1 && inputString[0] != 0x42) {
      inputIdx = 0;
    } else if (inputIdx == 2 && inputString[1] != 0x4d) {
      inputIdx = 0;
    } else if (inputIdx == 32) {
      inputIdx = 0;

      int temperature = round(bme.readTemperature() * 100);
      int humidity = round(bme.readHumidity() * 100);

      if (client.connected()) {
        Serial.swap();
        mqtt_publish("temperature", "%u.%.2u", temperature / 100, temperature % 100);
        mqtt_publish("humidity", "%u.%.2u", humidity / 100, humidity % 100);

        for(unsigned int i = 0; i < ARRAY_SIZE(pms5003_topics); i++) {
          byte high = inputString[4+i*2];
          byte low  = inputString[4+i*2+1];
          unsigned int value = (high << 8) | low;

          mqtt_publish(pms5003_topics[i], "%u", value);
        }
        Serial.flush();
        Serial.swap();
      }
    }
  }
}
