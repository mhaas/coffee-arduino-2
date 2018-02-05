#include "TempSensor.h"

TempSensor::TempSensor(SettingsStorage *_settings, MQTTLogger *_logger) {
  settings = _settings;
  logger = _logger;
}

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
    String rtdMessage = String("RTD: ") + String(rtd);
    logger->println(rtdMessage);

    float temperature = sensor->temperature(R_AT_ZERO_DEGREES, R_REFERENCE);

    String temperatureMessage = String("Temperature: ") + String(temperature);
    logger->println(temperatureMessage);
    // This is essentially a low-pass filter that can be helpful
    // for the D part in the PID.
    temperature = roundf(temperature * 100 ) / 100;

    String roundedMessage = String("Rounded temperature: ") + String(temperature);
    logger->println(roundedMessage);

    // TODO: what does roundf() return on NaN?
    if (isnan(rtd) || isnan(temperature)) {
      temperature = INVALID_READING;
      logger->println("TempSensor: rtd or temperature is NaN. Invalid reading detected!");
    }
    if (temperature < -50 || temperature > 200) {
      // Indicate that this is a bogus reading.
      temperature = INVALID_READING;
      logger->println("TempSensor: Invalid reading detected!");
    }
    uint8_t fault = sensor->readFault();
    if (fault != 0) {
      temperature = INVALID_READING;
      String faultMessage = String("TempSensor: fault detected!") + String(fault);
      logger->println(faultMessage);
      // TODO: does the Adafruit driver clear this bit?
      // I'd rather disable everything forever.
      // TODO: the Adafruit driver just reads the fault register without initiating
      // the fault detection cycle. This means the example on the Adafruit site is
      // not quite correct: some conditions checked there can only be
      // detected when a fault detection cycle is initiated.
    }
    settings->setCurrentTemperature(temperature);
  }
}
