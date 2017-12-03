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
    double rtd = sensor->readRTD();
    DEBUG.print("RTD: ");
    DEBUG.println(rtd);
    float temperature = sensor->temperature(R_AT_ZERO_DEGREES, R_REFERENCE);
    if (temperature < -50 || temperature > 200) {
      // Indicate that this is a bogus reading.
      temperature = INVALID_READING;
      DEBUG.println("TempSensor: Invalid reading detected!");
    }
    DEBUG.print("temperature:");
    DEBUG.println(temperature);
    settings->setCurrentTemperature(temperature);
  }
}
