#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <measurement.h>

const char * Measurement::topic(void) {
  const char * prefix = cfg->mqtt_prefix.c_str();
  if (last_prefix != prefix) {
    last_prefix = prefix;
    memset(full_topic, 0, 256);
    sprintf(full_topic, "%s/%s", last_prefix, name);
  }

  return full_topic;
}

void Measurement::broadcast(PubSubClient * client, char * formatted_value) {
  client->publish(topic(), formatted_value);
}

void Measurement::broadcast(HardwareSerial * serial, char * formatted_value) {
  serial->print(topic());
  serial->print(": ");
  serial->println(formatted_value);
}

void FloatMeasurement::record(float value) {
  last_float = value;
  last_measured_at = millis();
}

void FloatMeasurement::publish(PubSubClient * client) {
  char formatted_value[256];
  sprintf(formatted_value, "%.2f", last_float);
  broadcast(client, formatted_value);
}

void FloatMeasurement::publish(HardwareSerial * serial) {
  char formatted_value[256];
  sprintf(formatted_value, "%.2f", last_float);
  broadcast(serial, formatted_value);
}

void FloatMeasurement::publish(String &json) {
  json += "{\"name\": \"";
  json += name;
  json += "\", \"value\": ";
  json += last_float;
  json += ", \"last_measured_at\": ";
  json += last_measured_at;
  json += "}, ";
}

void UnsignedIntMeasurement::record(unsigned int value) {
  last_value = value;
  last_measured_at = millis();
}

void UnsignedIntMeasurement::publish(PubSubClient * client) {
  char formatted_value[256];
  sprintf(formatted_value, "%u", last_value);
  broadcast(client, formatted_value);
}

void UnsignedIntMeasurement::publish(HardwareSerial * serial) {
  char formatted_value[256];
  sprintf(formatted_value, "%u", last_value);
  broadcast(serial, formatted_value);
}

void UnsignedIntMeasurement::publish(String &json) {
  json += "{\"name\": \"";
  json += name;
  json += "\", \"value\": ";
  json += last_value;
  json += ", \"last_measured_at\": ";
  json += last_measured_at;
  json += "}, ";
}
