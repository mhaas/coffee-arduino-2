#include "TempSensor.h"

TempSensor::TempSensor(SettingsStorage *_settings) { settings = _settings; }

void TempSensor::begin(const byte _spiCsPin) {
  sensor = new Adafruit_MAX31865(_spiCsPin);
  // My PT100 has three wires
  sensor->begin(MAX31865_3WIRE);
}

void TempSensor::update() {
  // For PT100
  const int R_AT_ZERO_DEGREES = 100;
  // 430 Ohms for the Adafruit PT100 breakout
  const int R_REFERENCE = 430;
  // Sample every TEMP_INTERVALL ms
  int now = millis();
  if (now > lastRead + TEMP_INTERVALL) {
    lastRead = now;
    float temperature = sensor->temperature(R_AT_ZERO_DEGREES, R_REFERENCE);
    settings->setCurrentTemperature(temperature);
  }
}
