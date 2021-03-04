struct Config {
  String name;

  String mqtt_server;
  unsigned int mqtt_port;
  String mqtt_prefix;
  IPAddress address;
};

struct Measurement {
  const char *name;
  Config * cfg;

  unsigned long last_measured_at;
  const char * last_prefix;
  char full_topic[256];

  Measurement(const char *name, Config * cfg):
    name(name),
    cfg(cfg),
    last_measured_at(0),
    last_prefix(NULL)
    {};

  const char * topic(void);
  void broadcast(PubSubClient * client, char * formatted_value);
  void broadcast(HardwareSerial * serial, char * formatted_value);
};

struct FloatMeasurement : Measurement {
  float last_float;

  FloatMeasurement(const char *name, Config * cfg):
    Measurement(name, cfg),
    last_float(0.0)
    { };

  void record(float value);
  void publish(PubSubClient * client);
  void publish(HardwareSerial * serial);
  void publish(String &json);
};

struct UnsignedIntMeasurement : Measurement {
  unsigned int last_value;

  UnsignedIntMeasurement(const char *name, Config * cfg):
    Measurement(name, cfg),
    last_value(0)
    { };

  void record(unsigned int value);
  void publish(PubSubClient * client);
  void publish(HardwareSerial * serial);
  void publish(String &json);
};
