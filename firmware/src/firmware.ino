#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <PubSubClient.h>
#include <LittleFS.h>
#include <ArduinoOTA.h>

Adafruit_BME280 bme;

WiFiClient espClient;
PubSubClient client(espClient);
ESP8266WebServer server(80);

const char *mqtt_server = "10.0.1.211";
const char *mqtt_prefix = "home/test/esp8266aq";

byte inputString[32];
int inputIdx = 0;

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

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());

  Serial.println(myWiFiManager->getConfigPortalSSID());
}

String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
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
  json.reserve(512);
  json += "{\"measurements\": [";

  addMeasurementJson(json, measurement_temperature);
  addMeasurementJson(json, measurement_humidity);
  for(int i = 0; i < NUM_MEASUREMENTS_PMS5003; i++) {
    addMeasurementJson(json, measurements_pms5003[i]);
  }
  json.remove(json.lastIndexOf(",")); // remove trailing comma

  json += "], \"current_time\": ";
  json += millis();
  json += "}";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
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
  ArduinoOTA.begin();

  Serial.flush();
  client.setServer(mqtt_server, 1883);
  Serial.swap(); // Switch to sensor

  server.on("/status.json", webHandleStatus);
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

  String clientId = "ESP8266AQ-";
  clientId += String(random(0xffff), HEX);
  client.connect(clientId.c_str());
}

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

  server.handleClient();
  client.loop();
  server.handleClient();
  ArduinoOTA.handle();

  /* If there is more than one packet in the buffer we only want the most recent */
  while (Serial.available() > 32) {
    Serial.read();
    inputIdx = 0;
  }

  while (Serial.available()) {
    inputString[inputIdx] = Serial.read();
    inputIdx++;
    if (inputIdx == 1 && inputString[0] != 0x42) {
      inputIdx = 0;
    } else if (inputIdx == 2 && inputString[1] != 0x4d) {
      inputIdx = 0;
    } else if (inputIdx == 32) {
      inputIdx = 0;

      if (client.connected()) {
        Serial.swap();
        for(unsigned int i = 0; i < NUM_MEASUREMENTS_PMS5003; i++) {
          byte high = inputString[4+i*2];
          byte low  = inputString[4+i*2+1];
          unsigned int value = (high << 8) | low;

          measurements_pms5003[i].record(value);
        }
        Serial.flush();
        Serial.swap();
      }
    }
  }

  static unsigned long last_time_measurement = 0;
  if(millis() - last_time_measurement > 5000){
    last_time_measurement = millis();

    int temperature = round(bme.readTemperature() * 100);
    int humidity = round(bme.readHumidity() * 100);

    measurement_temperature.record(temperature);
    measurement_humidity.record(humidity);
  }
}
