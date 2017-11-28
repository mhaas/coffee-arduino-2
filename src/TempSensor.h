#ifndef TEMP_SENSOR_H

#define TEMP_SENSOR_H

#include "SettingsStorage.h"
#include <Adafruit_MAX31865.h>

class TempSensor {

public:
  TempSensor(SettingsStorage *_settings);
  void begin(const byte _spiCsPin);
  void update();

private:
  SettingsStorage *settings;
  int lastRead;
  Adafruit_MAX31865 *sensor;
};

#endif
