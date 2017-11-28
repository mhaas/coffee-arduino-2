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

    /** Set the window size for the time proportioning control.
 *  See the original PID_Relayoutput example.
 *
 *  The period of the signal is HEATER_WINDOW_SIZE ms (e.g. 5s) and
 *  the output of the PID is the duty cycle, e.g. how long the
 *  heater is one during that five second window.
 *  See: https://en.wikipedia.org/wiki/Duty_cycle
 *
 *  This is similar to a slow version of PWM.
 */

    const int HEATER_WINDOW_SIZE = 2000;

  /**
    * This variable holds the timestamp of the last started window.
    * This will be updated after a window has elapsed and a new window starts.
   */
    unsigned long windowStartTime;

    const int RELAY_ON = LOW;
    const int RELAY_OFF = HIGH;

    double pidOutput;
    Stream* dbgStream;
    int relayPin;

};

#endif
