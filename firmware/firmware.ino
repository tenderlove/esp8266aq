#include <SI7021.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

#define SDA 0 // GPIO0 on ESP-01 module
#define SCL 2 // GPIO2 on ESP-01 module

SI7021 sensor;

WiFiClient espClient;
PubSubClient client(espClient);

const char *mqtt_server = "192.168.1.7";
const char *mqtt_prefix = "home/livingroom/esp8266aq";

byte input_string[32];
int input_idx = 0;

void setup() {
  WiFiManager wifiManager;

  wifiManager.autoConnect();

  randomSeed(micros());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.begin(9600);
  Wire.begin(0, 2);
  sensor.begin(SDA, SCL);

  client.setServer(mqtt_server, 1883);
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

  while (Serial.available()) {
    input_string[input_idx++] = Serial.read();
    if (input_idx == 2) { // Check for start of packet
      if (!(input_string[0] == 0x42 && input_string[1] == 0x4d)) {
        input_idx = 0;
      }
    }
    if (input_idx == 32) {
      input_idx = 0;

      for(int i = 0; i < ARRAY_SIZE(pms5003_topics); i++) {
        byte high = input_string[4+i*2];
        byte low  = input_string[4+i*2+1];
        unsigned int value = (high << 8) | low;

        mqtt_publish(pms5003_topics[i], "%u", value);
      }
    }
  }

  static unsigned long last_time_measurement = 0;
  if(millis() - last_time_measurement > 5000){
    last_time_measurement = millis();

    auto weather = sensor.getHumidityAndTemperature();
    unsigned int temperature = weather.celsiusHundredths;
    unsigned int humidity = weather.humidityBasisPoints;

    mqtt_publish("temperature", "%u.%.2u", temperature / 100, temperature % 100);
    mqtt_publish("humidity",    "%u.%.2u", humidity    / 100, humidity    % 100);
  }
}
