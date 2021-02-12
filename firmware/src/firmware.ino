#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <PubSubClient.h>
#include <LittleFS.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>

Adafruit_BME280 bme;

WiFiClient espClient;
PubSubClient client(espClient);
ESP8266WebServer server(80);

#define LED_PIN 2
#define G2_PIN 14
#define G3_PIN 12

struct Measurement {
  const char *name;
  const int scale;

  unsigned long last_measured_at;
  int last_value;

  Measurement(const char *name, int scale = 0):
    name(name),
    scale(scale),
    last_measured_at(0),
    last_value(0) {};

  void record(int value) {
    last_value = value;
    last_measured_at = millis();
  }
};

Measurement measurement_temperature("temperature", 100);
Measurement measurement_humidity("humidity", 100);

#define NUM_MEASUREMENTS_PMS5003 12
Measurement measurements_pms5003[NUM_MEASUREMENTS_PMS5003] = {
  Measurement("pm1_0_standard"),
  Measurement("pm2_5_standard"),
  Measurement("pm10_standard"),
  Measurement("pm1_0_env"),
  Measurement("pm2_5_env"),
  Measurement("concentration_unit"),
  Measurement("particles_03um"),
  Measurement("particles_05um"),
  Measurement("particles_10um"),
  Measurement("particles_25um"),
  Measurement("particles_50um"),
  Measurement("particles_100um")
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

struct Config {
  String name;

  String mqtt_server;
  unsigned int mqtt_port;
  String mqtt_prefix;
};

Config config;

typedef StaticJsonDocument<512> ConfigJsonDocument;

static void loadConfigurationFromDoc(Config &config, ConfigJsonDocument &doc) {
  config.name = doc["name"].as<String>();
  config.mqtt_server = doc["mqtt_server"].as<String>();
  config.mqtt_port = doc["mqtt_port"].as<String>().toInt();
  config.mqtt_prefix = doc["mqtt_prefix"].as<String>();

  if (!config.mqtt_port) {
    config.mqtt_port = 1883;
  }
}

void loadConfiguration(Config &config) {
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

  /* Load defaults */
  config.name = "esp8266aq";
  config.mqtt_server = "";
  config.mqtt_port = 0;
  config.mqtt_prefix = "esp8266aq";
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

void addMeasurementJson(String &json, Measurement &m) {
  json += "{\"name\": \"";
  json += m.name;
  json += "\", \"value\": ";
  if (m.scale) {
    json += float(m.last_value) / m.scale;
  } else {
    json += m.last_value;
  }
  json += ", \"last_measured_at\": ";
  json += m.last_measured_at;
  json += "}, ";
}

void webHandleStatus() {
  String json;
  json.reserve(1024);
  json += "{\"measurements\": [";

  addMeasurementJson(json, measurement_temperature);
  addMeasurementJson(json, measurement_humidity);
  for(int i = 0; i < NUM_MEASUREMENTS_PMS5003; i++) {
    addMeasurementJson(json, measurements_pms5003[i]);
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
  LittleFS.begin();
  loadConfiguration(config);
  ArduinoOTA.begin();

  Serial.flush();
  Serial.swap(); // Switch to sensor

  server.on("/config.json", HTTP_POST, webHandleUpdateConfig);
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

  if (config.mqtt_server.length() && config.mqtt_port) {
    client.setServer(config.mqtt_server.c_str(), config.mqtt_port);

    String clientId = config.name;
    client.connect(clientId.c_str());
  }
}

void mqtt_publish(const char *topic, const char *format, ...) {
  char full_topic[256];
  char value[256];
  sprintf(full_topic, "%s/%s", config.mqtt_prefix.c_str(), topic);

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
  if (client.connected()) {
    client.loop();
  } else {
    mqtt_reconnect();
  }

  if (!client.connected()) {
    Serial.swap();
    Serial.println("MQTT client is not connected");
    Serial.flush();
    Serial.swap();
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
      Serial.swap();
      for(int i = 0; i < NUM_MEASUREMENTS_PMS5003; i++) {
        unsigned int value = pms5003.get_value(i);

        measurements_pms5003[i].record(value);
        mqtt_publish(measurements_pms5003[i].name, "%u", value);
      }
      Serial.flush();
      Serial.swap();
    }
  }

  static unsigned long last_time_measurement = 0;
  if(millis() - last_time_measurement > 5000){
    last_time_measurement = millis();

    int temperature = round(bme.readTemperature() * 100);
    int humidity = round(bme.readHumidity() * 100);

    measurement_temperature.record(temperature);
    measurement_humidity.record(humidity);
    mqtt_publish("temperature", "%u.%.2u", temperature / 100, temperature % 100);
    mqtt_publish("humidity",    "%u.%.2u", humidity    / 100, humidity    % 100);
  }
}
