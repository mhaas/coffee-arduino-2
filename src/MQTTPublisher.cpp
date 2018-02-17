#include "MQTTPublisher.h"

MQTTPublisher::MQTTPublisher(SettingsStorage *settings) {
  this->_settings = settings;
 _mqttClient = PubSubClient(_espClient);
}

void MQTTPublisher::begin(const char* mqttServer, int mqttPort) {
  _espClient.setTimeout(MQTT_TIMEOUT);
  _mqttClient.setServer(mqttServer, mqttPort);
  // ensure that we publish/connect as soon as possible, and not after the uptime
  // exceeds MQTT_PUBLISH_CYCLE/MQTT_RECONNECT_CYCLE
  _lastUpdateTime = MQTT_PUBLISH_CYCLE;
  _lastReconnectAttempt = MQTT_RECONNECT_CYCLE;
  if (!this->reconnectIfNecessary()) {
    DEBUG.println("MQTTPublisher: Initial connection attempt failed. Will keep trying.");
  }
}

boolean MQTTPublisher::reconnectIfNecessary() {
  if (!this->_mqttClient.connected()) {
    long now = millis();
    // Attempt to connect every 5s if we are not connected
    if (now - this->_lastReconnectAttempt > MQTT_RECONNECT_CYCLE) {
      this->_lastReconnectAttempt = now;
      this->reconnect();
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
  String topic = String("/pid");
  String json;
  this->_settings->toJSON(json);
  this->publishStringToSubTopic(topic, json);
}

void MQTTPublisher::log(String& logMessage) {
  String topic = String("/log_string");
  DEBUG.println(logMessage);
  this->publishStringToSubTopic(topic, logMessage);
}

void MQTTPublisher::publishStringToSubTopic(String& subTopic, String& message) {
  if (!_mqttClient.connected()) {
    DEBUG.println("Failed to publish to MQTT: client not connected!");
    return;
  }
  String topic = String("/") + String(NODE_NAME) + subTopic;
  boolean result = _mqttClient.publish(topic.c_str(), message.c_str());
  if (! result) {
    DEBUG.println("Failed to publish to MQTT!");
  }
}
