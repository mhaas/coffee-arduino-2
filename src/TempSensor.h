#ifndef TEMP_SENSOR_H

#define TEMP_SENSOR_H

#include "SettingsStorage.h"
#include <Adafruit_MAX31865.h>

class TempSensor {

public:
  TempSensor(SettingsStorage *_settings);
  //~TempSensor();
  void begin(const byte _spiCsPin);
  void update();

private:
  SettingsStorage *settings;
  int lastRead;
  const int TEMP_INTERVALL = 500;
  Adafruit_MAX31865 *sensor;
};

#endif
