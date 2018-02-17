#ifndef TEMP_SENSOR_H

#define TEMP_SENSOR_H

#include <Adafruit_MAX31865.h>

#include "MQTTLogger.h"
#include "SettingsStorage.h"

class TempSensor {

public:
  TempSensor(SettingsStorage *_settings, MQTTLogger *_logger);
  void begin(const byte _spiCsPin);
  void update();

  static const int INVALID_READING = -999;

private:
  SettingsStorage *settings;
  MQTTLogger *logger;
  int lastRead;
  Adafruit_MAX31865 *sensor;
};

#endif
