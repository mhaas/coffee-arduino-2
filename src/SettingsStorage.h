#ifndef SETTINGS_STORAGE_H
#define SETTINGS_STORAGE_H

#include "Arduino.h"


const size_t CONFIG_VERSION_LENGTH = 4;

class SettingsStorage {

  public:
    void begin();
    void update();

    void setKp(double _kp);
    void setKi(double _ki);
    void setKd(double _kd);
    void setDesiredTemperature(double _desiredTemp);
    void setTempOffset(double _tempOffset);
    void setCurrentTemperature(double currentTemperature);
    double* getDesiredTemperature();
    double getTempOffset();
    double getKp();
    double getKi();
    double getKd();
    double* getCurrentTemperature();


    struct PidSettings {
      double desiredTemperature;
      double tempOffset;
      double kp;
      double ki;
      double kd;
      char configVersion[CONFIG_VERSION_LENGTH];
    };

  private:
    const double DEFAULT_HEATER_KP = 2;
    const double DEFAULT_HEATER_KI = 5;
    const double DEFAULT_HEATER_KD = 1;

    const char CONFIG_VERSION[CONFIG_VERSION_LENGTH] = "124";
    // At which byte of the EEPROM the config data is stored
    // If you change this, make sure to change the size argument
    // to EEPROM.begin as well.
    const size_t CONFIG_DATA_OFFSET = 0;
    const double DEFAULT_HEATER_TEMP = 50;

    double currentTemperature = 0;

    SettingsStorage::PidSettings storage = {
      .desiredTemperature = DEFAULT_HEATER_TEMP,
      .tempOffset = 0,
      .kp = DEFAULT_HEATER_KP,
      .ki = DEFAULT_HEATER_KI,
      .kd = DEFAULT_HEATER_KD
      };

    void store(double value, double* target);


};


#endif
