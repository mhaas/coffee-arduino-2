#ifndef HEATER_PID_H

#define HEATER_PID_H

#include <Arduino.h>
#include "SettingsStorage.h"
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>
#include <ESP8266WebServer.h>



class HeaterPID {
  public:
    HeaterPID(SettingsStorage* settings);
    void begin(const int _relayPin);
    void update();
    void end();
    void triggerAutoTune();
    ESP8266WebServer::THandlerFunction getTriggerAutoTuneCallback();

    static const int MAX_TEMPERATURE_ALLOWED = 130;

  private:
    PID* pid;
    PID_ATune* aTune;
    SettingsStorage* settings;
    bool autoTuneRequested;
    bool turnedOff;

  /**
    * This variable holds the timestamp of the last started window.
    * This will be updated after a window has elapsed and a new window starts.
   */
    unsigned long windowStartTime;

    double pidOutput;
    int relayPin;

    void enableHeater();
    void disableHeater();
    boolean checkSanity();

};

#endif
