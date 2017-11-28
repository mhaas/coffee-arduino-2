#ifndef HEATER_PID_H

#define HEATER_PID_H

#include <Arduino.h>
#include "SettingsStorage.h"
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>



class HeaterPID {
  public:
    HeaterPID(SettingsStorage* settings);
    void begin(const int _relayPin);
    void update();
    void triggerAutoTune();

  private:
    PID* pid;
    PID_ATune* aTune;
    SettingsStorage* settings;
    bool autoTuneRequested;

  /**
    * This variable holds the timestamp of the last started window.
    * This will be updated after a window has elapsed and a new window starts.
   */
    unsigned long windowStartTime;

    double pidOutput;
    int relayPin;

    void enableHeater();
    void disableHeater();

};

#endif
