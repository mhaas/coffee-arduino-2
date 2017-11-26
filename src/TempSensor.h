#ifndef TEMP_SENSOR_H

#define TEMP_SENSOR_H

/*
 * Initialize the temperature sensor library.
 */

#include "SettingsStorage.h"


class TempSensor {

  public:
    TempSensor(SettingsStorage* _settings);
    //~TempSensor();
    void begin(const byte _spiCsPin, const double _analogReference);
    void update();

  private:
    SettingsStorage* settings;
    int lastRead;
    const int TEMP_INTERVALL = 500;

};

#endif
