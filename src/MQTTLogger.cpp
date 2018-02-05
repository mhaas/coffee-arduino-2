#include "MQTTLogger.h"

MQTTLogger::MQTTLogger(MQTTPublisher* publisher) {
  this->_publisher = publisher;
}

void MQTTLogger::print(String& message) {
  this->println(message);
}

void MQTTLogger::print(const char* message) {
  this->println(message);
}

void MQTTLogger::println(String& message) {
  this->_publisher->log(message);
}

void MQTTLogger::println(const char* message) {
  String msg = String(message);
  this->_publisher->log(msg);
}

void MQTTLogger::println(float message) {
  String msg = String(message);
  this->_publisher->log(msg);
}

void MQTTLogger::println(double message) {
  String msg = String(message);
  this->_publisher->log(msg);
}

void MQTTLogger::println(int message) {
  String msg = String(message);
  this->_publisher->log(msg);
}
