#ifndef MQTT_PUBLISHER_H

#define MQTT_PUBLISHER_H

#include "SettingsStorage.h"
#include <WiFiClient.h>
#include <PubSubClient.h>

// milliseconds
#define MQTT_PUBLISH_CYCLE 500

// How long to wait e.g. on connect()
// Default is 5s, and we don't want to block the HeaterPID that long
#define MQTT_TIMEOUT 200


// If we can't establish a connection to the broker (or we lose it),
// attempt to connect every X ms.
// Note that this can block up to 2 * MQTT_TIMEOUT, for the DNS lookup
// and actual connect attempt.
#define MQTT_RECONNECT_CYCLE 5000

class MQTTPublisher {

public:
  MQTTPublisher(SettingsStorage *_settings);
  void begin(const char* mqttServer, int port);
  void update();

private:
  SettingsStorage *_settings;
  WiFiClient _espClient;
  PubSubClient _mqttClient;
  long _lastReconnectAttempt = 0;
  long _lastUpdateTime = 0;
  boolean reconnect();
  boolean reconnectIfNecessary();
  void publish();
};

#endif
