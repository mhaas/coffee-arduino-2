
// Default packet size is 128,
// whichc is not enough for our JSON payload.
#define MQTT_MAX_PACKET_SIZE 4096
#include "MQTTPublisher.h"

MQTTPublisher::MQTTPublisher(SettingsStorage *_settings) {
  _settings = _settings;
 _mqttClient = PubSubClient(_espClient);
}

void MQTTPublisher::begin(char* mqttServer, int mqttPort) {
  // The default timeout is 5000ms, which is too long: we do not want to
  // block the HeaterPID loop this long.
  // Worst case is one timeout period for the DNS lookup and one timeout
  // period for the actual connect.
  _espClient.setTimeout(300);
  _mqttClient.setServer(mqttServer, mqttPort);
  if (!this->reconnect()) {
    DEBUG.println("MQTTPublisher: Initial connection attempt failed. Will keep trying.");
  }
}

boolean MQTTPublisher::reconnectIfNecessary() {
  if (!this->_mqttClient.connected()) {
    long now = millis();
    // Attempt to connect every 5s if we are not connected
    if (now - this->_lastReconnectAttempt > 5000) {
      this->_lastReconnectAttempt = now;
      if (this->reconnect()) {
         this->_lastReconnectAttempt = 0;
      }
    }
  }
  return this->_mqttClient.connected();
}


boolean MQTTPublisher::reconnect() {
  if (_mqttClient.connect(NODE_NAME)) {
    DEBUG.println("MQTT connected!");
  }
  return _mqttClient.connected();
}


void MQTTPublisher::update() {
  boolean connected = this->reconnectIfNecessary();
  if (connected) {
    this->publish();
  }
}

void MQTTPublisher::publish() {
  String topic = String("/") + String(NODE_NAME) + String("/pid");
  boolean result = _mqttClient.publish(topic.c_str(), this->_settings->toJSON().c_str());
  if (! result) {
    DEBUG.println("Failed to publish to MQTT!");
  }
}
