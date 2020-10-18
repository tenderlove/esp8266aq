#include <SI7021.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <LittleFS.h>
#include <ArduinoOTA.h>

#define SDA 0 // GPIO0 on ESP-01 module
#define SCL 2 // GPIO2 on ESP-01 module

SI7021 sensor;

WiFiClient espClient;
PubSubClient client(espClient);
ESP8266WebServer server(80);

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

const char *mqtt_server = "192.168.1.7";
const char *mqtt_prefix = "home/outside/esp8266aq";

byte input_string[32];
int input_idx = 0;

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

  server.send(200, "application/json", json);
}

void setup() {
  WiFiManager wifiManager;

  wifiManager.autoConnect();

  randomSeed(micros());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  LittleFS.begin();
  Serial.begin(9600);
  Wire.begin(0, 2);
  sensor.begin(SDA, SCL);
  ArduinoOTA.begin();

  client.setServer(mqtt_server, 1883);

  server.on("/status.json", webHandleStatus);
  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "404: Not Found");
  });

  server.begin();
}

void mqtt_reconnect() {
  while (!client.connected()) {
    // Create a random client ID
    String clientId = "ESP8266AQ-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqtt_publish(const char *topic, const char *format, ...) {
  if (!client.connected())
    return;

  char full_topic[256];
  char value[256];
  sprintf(full_topic, "%s/%s", mqtt_prefix, topic);

  va_list args;
  va_start(args, format);
  vsprintf(value, format, args);
  va_end(args);

  client.publish(full_topic, value, true);
}

void loop() {
  if (!client.connected()) {
    mqtt_reconnect();
  }
  client.loop();

  server.handleClient();
  ArduinoOTA.handle();

  /* If there is more than one packet in the buffer we only want the most recent */
  while (Serial.available() > 32) {
    Serial.read();
    input_idx = 0;
  }

  while (Serial.available()) {
    input_string[input_idx++] = Serial.read();
    if (input_idx == 1 && input_string[0] != 0x42) {
      input_idx = 0;
    } else if (input_idx == 2 && input_string[1] != 0x4d) {
      input_idx = 0;
    } else if (input_idx == 32) {
      input_idx = 0;

      for(int i = 0; i < NUM_MEASUREMENTS_PMS5003; i++) {
        byte high = input_string[4+i*2];
        byte low  = input_string[4+i*2+1];
        unsigned int value = (high << 8) | low;

        measurements_pms5003[i].record(value);
        //mqtt_publish(pms5003_topics[i], "%u", value);
      }
    }
  }

  static unsigned long last_time_measurement = 0;
  if(millis() - last_time_measurement > 5000){
    last_time_measurement = millis();

    auto weather = sensor.getHumidityAndTemperature();
    int temperature = weather.celsiusHundredths;
    unsigned int humidity = weather.humidityBasisPoints;

    measurement_temperature.record(temperature);
    measurement_humidity.record(humidity);
    //mqtt_publish("temperature", "%u.%.2u", temperature / 100, temperature % 100);
    //mqtt_publish("humidity",    "%u.%.2u", humidity    / 100, humidity    % 100);
  }
}
