
#ifndef MQTT_LOGGER_H

#define MQTT_LOGGER_H

#include <Arduino.h>

#include "MQTTPublisher.h"

class MQTTLogger {

public:
  MQTTLogger(MQTTPublisher *publisher);

  void print(String &message);
  void print(const char *message);
  void println(String &message);
  void println(const char *message);
  void println(float message);
  void println(double message);
  void println(int message);

private:
  MQTTPublisher *_publisher;
};

#endif
