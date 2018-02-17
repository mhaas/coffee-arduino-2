#include "MQTTPublisher.h"

MQTTPublisher::MQTTPublisher(SettingsStorage *settings) {
  this->_settings = settings;
}

void MQTTPublisher::begin(const char *mqttServer, int mqttPort) {
  _mqttClient.setServer(mqttServer, mqttPort);
  _mqttClient.setClientId(NODE_NAME);
  // ensure that we publish/connect as soon as possible, and not after the
  // uptime
  // exceeds MQTT_PUBLISH_CYCLE/MQTT_RECONNECT_CYCLE
  _lastUpdateTime = MQTT_PUBLISH_CYCLE;
  _lastReconnectAttempt = MQTT_RECONNECT_CYCLE;
}

void MQTTPublisher::update() {
  if (millis() - _lastUpdateTime < MQTT_PUBLISH_CYCLE) {
    return;
  }
  if (_mqttClient.connected()) {
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

void MQTTPublisher::log(String &logMessage) {
  String topic = String("/log_string");
  DEBUG.println(logMessage);
  this->publishStringToSubTopic(topic, logMessage);
}

void MQTTPublisher::publishStringToSubTopic(String &subTopic, String &message) {
  if (!_mqttClient.connected()) {
    DEBUG.println("Failed to publish to MQTT: client not connected!");
    return;
  }
  String topic = String("/") + String(NODE_NAME) + subTopic;
  _mqttClient.publish(topic.c_str(), 0, false, message.c_str());
}
