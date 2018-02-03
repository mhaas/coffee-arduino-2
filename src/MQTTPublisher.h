#ifndef MQTT_PUBLISHER_H

#define MQTT_PUBLISHER_H

#include "SettingsStorage.h"
#include <WiFiClient.h>
#include <PubSubClient.h>

class MQTTPublisher {

public:
  MQTTPublisher(SettingsStorage *_settings);
  void begin(char* mqttServer, int port);
  void update();

private:
  SettingsStorage *_settings;
  WiFiClient _espClient;
  PubSubClient _mqttClient;
  long _lastReconnectAttempt;
  boolean reconnect();
  boolean reconnectIfNecessary();
  void publish();
};

#endif
