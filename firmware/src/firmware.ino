#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <PubSubClient.h>
#include <LittleFS.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <math.h>
#include <ESP8266mDNS.h>
#include <measurement.h>

#ifndef NODE_NAME
#define NODE_NAME "esp8266aq"
#endif

#ifndef MQTT_SERVER
#define MQTT_SERVER ""
#endif

#ifndef MQTT_PORT
#define MQTT_PORT 1833
#endif

#ifndef MQTT_PREFIX
#define MQTT_PREFIX "esp8266aq"
#endif

#define LED_PIN 2
#define G2_PIN 14
#define G3_PIN 12

Adafruit_BME280 bme;

WiFiClient espClient;
PubSubClient client(espClient);
ESP8266WebServer server(80);

Config config;

FloatMeasurement measurement_temperature("temperature", &config);
FloatMeasurement measurement_humidity("humidity", &config);
FloatMeasurement measurement_dewpoint("dewpoint", &config);

#define NUM_MEASUREMENTS_PMS5003 12
UnsignedIntMeasurement measurements_pms5003[NUM_MEASUREMENTS_PMS5003] = {
  UnsignedIntMeasurement("pm1_0_standard", &config),
  UnsignedIntMeasurement("pm2_5_standard", &config),
  UnsignedIntMeasurement("pm10_standard", &config),
  UnsignedIntMeasurement("pm1_0_env", &config),
  UnsignedIntMeasurement("pm2_5_env", &config),
  UnsignedIntMeasurement("concentration_unit", &config),
  UnsignedIntMeasurement("particles_03um", &config),
  UnsignedIntMeasurement("particles_05um", &config),
  UnsignedIntMeasurement("particles_10um", &config),
  UnsignedIntMeasurement("particles_25um", &config),
  UnsignedIntMeasurement("particles_50um", &config),
  UnsignedIntMeasurement("particles_100um", &config)
};

struct PMS5003 {
  byte input_string[32];
  int input_idx;

  PMS5003() : input_idx(0) {}

  void process_byte(byte in) {
    if(input_idx == 32) {
      input_idx = 0;
    }

    input_string[input_idx++] = in;
    if (input_idx == 1 && input_string[0] != 0x42) {
      input_idx = 0;
    } else if (input_idx == 2 && input_string[1] != 0x4d) {
      input_idx = 0;
    }
  }

  void reset_input() {
    input_idx = 0;
  }

  unsigned int get_value(int idx) {
    byte high = input_string[4 + idx * 2];
    byte low  = input_string[4 + idx * 2 + 1];
    return (high << 8) | low;
  }

  /* Are we a full, valid packet */
  bool is_valid_packet() {
    if (input_idx != 32)
      return false;

    /* TODO: check crc */

    return true;
  }
};

PMS5003 pms5003;

typedef StaticJsonDocument<512> ConfigJsonDocument;

static void loadConfigurationFromDoc(Config &config, ConfigJsonDocument &doc) {
  if (doc["mqtt_server"])
    config.mqtt_server = doc["mqtt_server"].as<String>();

  if (doc["mqtt_port"])
    config.mqtt_port = doc["mqtt_port"].as<String>().toInt();

  if (doc["mqtt_prefix"])
    config.mqtt_prefix = doc["mqtt_prefix"].as<String>();
}

void loadConfiguration(Config &config) {
  /* Load defaults */
  config.name = NODE_NAME;
  config.mqtt_server = MQTT_SERVER;
  config.mqtt_port = MQTT_PORT;
  config.mqtt_prefix = MQTT_PREFIX;

  if (LittleFS.exists("config.json")) {
    File file = LittleFS.open("config.json", "r");

    ConfigJsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);

    if (!error) {
      loadConfigurationFromDoc(config, doc);
      return;
    }

    file.close();
  }

}

void configModeCallback(WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".json")) return "application/json";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

bool handleFileRead(String path) {
  if (path.endsWith("/"))
    path += "index.html";

  String contentType = getContentType(path);
  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }

  return false;
}

void webHandleStatus() {
  String json;
  json.reserve(1024);
  json += "{\"measurements\": [";

  measurement_temperature.publish(json);
  measurement_humidity.publish(json);
  for(int i = 0; i < NUM_MEASUREMENTS_PMS5003; i++) {
    measurements_pms5003[i].publish(json);
  }
  json.remove(json.lastIndexOf(",")); // remove trailing comma

  json += "], \"current_time\": ";
  json += millis();
  json += ", \"mqtt\": {\"connected\": ";
  json += client.connected() ? "true" : "false";
  json += ", \"server\": \"";
  json += config.mqtt_server;
  json += "\", \"port\": ";
  json += config.mqtt_port;
  json += "}, \"esp8266\": {\"heap_free\":";
  json += ESP.getFreeHeap();
  json += ", \"heap_fragmentation\": ";
  json += ESP.getHeapFragmentation();
  json += ", \"heap_max_free_block_size\": ";
  json += ESP.getMaxFreeBlockSize();
  json += "}}";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

void webHandleUpdateConfig() {
  String body = server.arg("plain");

  File file = LittleFS.open("config.json", "w");
  file.print(body);
  file.close();

  server.send(200, "text/json", "{\"success\":true}" );

  ESP.restart();
}

void webHandleReadConfig() {
  String json;
  json.reserve(1024);

  json += "{\"name\": \"";
  json += config.name;
  json += "\", \"mqtt_server\": \"";
  json += config.mqtt_server;
  json += "\", \"mqtt_port\": \"";
  json += config.mqtt_port;
  json += "\", \"mqtt_prefix\": \"";
  json += config.mqtt_prefix;
  json += "\"}";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

void setup() {
  WiFiManager wifiManager;

  pinMode(G2_PIN, INPUT);
  pinMode(G3_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(2, HIGH);

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
  LittleFS.begin();
  loadConfiguration(config);
  Serial.print("Node name: ");
  Serial.println(config.name);
  Serial.print("MQTT Server: ");
  Serial.println(config.mqtt_server);
  Serial.print("MQTT Port: ");
  Serial.println(config.mqtt_port);
  Serial.print("MQTT Prefix: ");
  Serial.println(config.mqtt_prefix);
  Serial.print("GPIO: ");
  Serial.print(digitalRead(G2_PIN));
  Serial.println(digitalRead(G3_PIN));

  MDNS.begin(config.name);

  ArduinoOTA.begin();

  Serial.flush();
  Serial.swap(); // Switch to sensor

  server.on("/config.json", HTTP_POST, webHandleUpdateConfig);
  server.on("/config.json", HTTP_GET, webHandleReadConfig);
  server.on("/status.json", HTTP_GET, webHandleStatus);
  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "404: Not Found");
  });

  server.begin();
}

void mqtt_reconnect() {
  static unsigned long last_attempt = 0;
  if (millis() - last_attempt < 5000) {
    return;
  }

  last_attempt = millis();
  Serial.swap();
  if (config.mqtt_server.length() && config.mqtt_port) {
    if (WiFi.hostByName(config.mqtt_server.c_str(), config.address)) {
      client.setServer(config.address, config.mqtt_port);
      String clientId = config.name;
      if(client.connect(clientId.c_str())) {
        Serial.println("Connected\n");
      } else {
        Serial.println(clientId);
        Serial.println(config.address);
        Serial.print("failed, rc=");
        Serial.println(client.state());
      }
    } else {
      Serial.print("Couldn't find address: ");
      Serial.println(config.mqtt_server);
    }

  }
  Serial.flush();
  Serial.swap();
}

void readI2CSensors(void) {
  static unsigned long last_time_measurement = 0;

  if(millis() - last_time_measurement > 1000){
    last_time_measurement = millis();

    float temp = bme.readTemperature();
    float rh = bme.readHumidity();

    // https://bmcnoldy.rsmas.miami.edu/humidity_conversions.pdf
    float x = (17.625 * temp) / (243.04 + temp);
    float l = log(rh / 100.0);
    float dewpoint = 243.04 * (l + x) / (17.625 - l - x);

    measurement_temperature.record(temp);
    measurement_humidity.record(rh);
    measurement_dewpoint.record(dewpoint);

    measurement_temperature.publish(&client);
    measurement_humidity.publish(&client);
    measurement_dewpoint.publish(&client);

    // Publish to the serial client if G2 pin is HIGH
    if (digitalRead(G2_PIN) == HIGH) {
      Serial.flush();
      Serial.swap();
      measurement_temperature.publish(&Serial);
      measurement_humidity.publish(&Serial);
      measurement_dewpoint.publish(&Serial);
      Serial.flush();
      Serial.swap();
    }
  }
}

void loop() {
  if (client.connected()) {
    client.loop();
  } else {
    mqtt_reconnect();
  }

  if (client.connected()) {
    digitalWrite(LED_PIN, LOW); // LOW == ON
  } else {
    digitalWrite(LED_PIN, HIGH);
  }

  server.handleClient();
  ArduinoOTA.handle();

  /* If there is more than one packet in the buffer we only want the most recent */
  while (Serial.available() > 32) {
    Serial.read();
    pms5003.reset_input();
  }

  while (Serial.available()) {
    pms5003.process_byte(Serial.read());

    if (pms5003.is_valid_packet()) {
      if (!client.connected()) {
        Serial.swap();
        Serial.println("MQTT client is not connected");
        Serial.flush();
        Serial.swap();
      }

      for(int i = 0; i < NUM_MEASUREMENTS_PMS5003; i++) {
        unsigned int value = pms5003.get_value(i);

        measurements_pms5003[i].record(value);
        measurements_pms5003[i].publish(&client);
      }

      // Publish to the serial client if G2 pin is HIGH
      if (digitalRead(G2_PIN) == HIGH) {
        Serial.flush();
        Serial.swap();
        for(int i = 0; i < NUM_MEASUREMENTS_PMS5003; i++) {
          measurements_pms5003[i].publish(&Serial);
        }
        Serial.flush();
        Serial.swap();
      }
    }
  }

  readI2CSensors();
}
