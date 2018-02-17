#ifndef HEATER_PID_H

#define HEATER_PID_H

#include <Arduino.h>
#include <PID_v1.h>
#include <functional>
#include <Average.h>

#include "SettingsStorage.h"
// To get INVALID_READING
#include "MQTTLogger.h"
#include "TempSensor.h"


class HeaterPID {
public:
  HeaterPID(SettingsStorage *settings, MQTTLogger *logger);
  void begin(const int _relayPin);
  void update();
  void end();

  static const int MAX_TEMPERATURE_ALLOWED = 130;

private:
  PID *pid;
  SettingsStorage *settings;
  MQTTLogger *logger;
  bool turnedOff;

  /**
    * This variable holds the timestamp of the last started window.
    * This will be updated after a window has elapsed and a new window starts.
   */
  unsigned long windowStartTime;

  /**
   * Updated every loop.
   */
  unsigned long lastLoopStartTime;

  Average<unsigned long> *timesBetweenLoops;

  double pidOutput;
  int relayPin;

  void enableHeater();
  void disableHeater();
  boolean checkSanity();
  void turnOffPIDAlgorithm();
};

#endif
