#include "MQTTPublisher.h"

MQTTPublisher::MQTTPublisher(SettingsStorage *_settings) {
  _settings = _settings;
 _mqttClient = PubSubClient(_espClient);
}

void MQTTPublisher::begin(const char* mqttServer, int mqttPort) {
  _espClient.setTimeout(MQTT_TIMEOUT);
  _mqttClient.setServer(mqttServer, mqttPort);
  if (!this->reconnect()) {
    DEBUG.println("MQTTPublisher: Initial connection attempt failed. Will keep trying.");
  }
}

boolean MQTTPublisher::reconnectIfNecessary() {
  if (!this->_mqttClient.connected()) {
    long now = millis();
    // Attempt to connect every 5s if we are not connected
    if (now - this->_lastReconnectAttempt > MQTT_RECONNECT_CYCLE) {
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
  _mqttClient.loop();
  if (millis() - _lastUpdateTime <  MQTT_PUBLISH_CYCLE) {
    return;
  }
  // reconnectIfNecessary
  boolean connected = this->reconnectIfNecessary();
  if (connected) {
    this->publish();
  }
  _lastUpdateTime = millis();
}

void MQTTPublisher::publish() {
  String topic = String("/") + String(NODE_NAME) + String("/pid");
  boolean result = _mqttClient.publish(topic.c_str(), this->_settings->toJSON().c_str());
  if (! result) {
    DEBUG.println("Failed to publish to MQTT!");
  }
}
