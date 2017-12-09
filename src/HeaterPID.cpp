#include "HeaterPID.h"
// To get INVALID_READING
#include "TempSensor.h"

HeaterPID::HeaterPID(SettingsStorage *_settings) {
  settings = _settings;
  autoTuneRequested = false;
}

void HeaterPID::begin(const int _relayPin) {
  relayPin = _relayPin;
  pinMode(relayPin, OUTPUT);
  disableHeater();

  pid = new PID(settings->getCurrentTemperature(), &pidOutput,
                settings->getDesiredTemperature(), settings->getKp(),
                settings->getKi(), settings->getKd(), DIRECT);

  windowStartTime = millis();

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
  // We can switch every 20ms: 1000 * 50Hz
  pid->SetOutputLimits(0, HEATER_WINDOW_SIZE);

  // turn the PID on
  // TODO: initialize the temperature sensor variables
  pid->SetMode(AUTOMATIC);
}

void HeaterPID::update() {

  if (!this->checkSanity()) {
    DEBUG.println("HeaterPID: failed sanity check. Disabling heater!");
    this->disableHeater();
    return;
  }

  if (this->turnedOff) {
    DEBUG.println("HeaterPID::update: turnedOff is true! Disabling heater");
    this->disableHeater();
    return;
  }

  if (autoTuneRequested) {
    if (aTune->Runtime() != 0) {
      // done auto-tuning
      autoTuneRequested = false;
      double kp = aTune->GetKp();
      double ki = aTune->GetKi();
      double kd = aTune->GetKd();
      DEBUG.println("Autotune done.");
      DEBUG.print("kp: ");
      DEBUG.println(kp);
      DEBUG.print("ki: ");
      DEBUG.println(ki);
      DEBUG.print("kd: ");
      DEBUG.println(kd);
      pid->SetTunings(kp, ki, kd);
      settings->setKp(kp);
      settings->setKi(ki);
      settings->setKd(kd);
      free(aTune);
    }
  }


  // Duration of one AC period. 20ms for 50Hz
  const double PERIOD_DURATION = 20.0;


  // How many milliseconds have passed since we have started the current window?
  float currentWindowElapsed = millis() - windowStartTime;

  if (currentWindowElapsed > HEATER_WINDOW_SIZE) {
    // time to shift the Relay Window
    windowStartTime = millis();
    // We only switch on once and off once during HEATER_WINDOW_SIZE.
    // For now, we only compute the PID output once during that period:
    // But we can compute it more often and then adjust the heating duration
    // as long as it does not cause additional switches
    pid->Compute();
    DEBUG.print("Starting new window.");
    DEBUG.print("PID says: ");
    DEBUG.println(pidOutput);
    // We always switch after a number of full periods. If we do not respect
    // the period, the SSR may turn off after an uneven number of half-periods,
    // which can lead to noise in the power grid.
    // See again the TAB on the FH stralsund server, although the
    // "Unsymmetrische Schwingungspaketsteuerungen" mentioned there may refer
    // to continously switching only one half-period
    // Also see https://de.wikipedia.org/wiki/Schwingungspaketsteuerung
    double roundedPidOutput = floor(pidOutput  / PERIOD_DURATION) * PERIOD_DURATION;
    settings->setPidOutput(roundedPidOutput);
    // TODO: for this logic to work, the function must be called every millisecond
    // If we know an offset, we should subtract that here
    DEBUG.print("PID output rounded to grid frequency:");
    DEBUG.println(roundedPidOutput);
    DEBUG.print("Desired Temp:");
    DEBUG.println(*(settings->getDesiredTemperature()));
  }

  // Now decide if we are in the duty cycle of our period, i.e. if we want
  // to turn the relay for the header on or off
  if (currentWindowElapsed > pidOutput) {
    disableHeater();
  } else {
    enableHeater();
  }
}

void HeaterPID::enableHeater() {
  digitalWrite(relayPin, HEATER_RELAY_ON);
}

void HeaterPID::disableHeater() {
  digitalWrite(relayPin, HEATER_RELAY_OFF);
}

void HeaterPID::triggerAutoTune() {
  DEBUG.println("triggerAutoTune!");
  autoTuneRequested = true;

  aTune = new PID_ATune(settings->getCurrentTemperature(),
                        settings->getDesiredTemperature());
  // 0: PI, 1: PID
  aTune->SetControlType(0);
  // TODO: there are parameters which look interesting, like
  // SetNoiseBand and SetLookbackSec. The defaults look useful.
}

/**
* Performs some checks if observed and desired values look sane. Returns false
* if something bad is observed, e.g. temperature too high.
*/
boolean HeaterPID::checkSanity() {
  if (HEATER_WINDOW_SIZE < 600) {
    // See Table 2 here: http://antriebstechnik.fh-stralsund.de/1024x768/Dokumentenframe/Kompendium/TAB/TAB.htm
    // For the limits
    // This table is not in the current TAB, it seems, but should still be relevant
    DEBUG.println("HeaterPID: HEATER_WINDOW_SIZE too slow. Switching this often puts noise on the power grid.");
    return false;
  }
  double currentTemperature = *settings->getCurrentTemperature();
  if (currentTemperature == TempSensor::INVALID_READING) {
    DEBUG.println("HeaterPID: temperature is INVALID_READING!");
    return false;
  }
  if (currentTemperature > MAX_TEMPERATURE_ALLOWED) {
    DEBUG.println("HeaterPID: observed temperature higher than MAX_TEMPERATURE_ALLOWED");
    return false;
  }
  double desiredTemperature = *settings->getDesiredTemperature();
  if (desiredTemperature > MAX_TEMPERATURE_ALLOWED) {
    DEBUG.println("HeaterPID: desired temperature higher than MAX_TEMPERATURE_ALLOWED");
    return false;
  }
  if (desiredTemperature < 0) {
    DEBUG.println("HeaterPID: desired temperatur < 0!");
    return false;
  }
  return true;
}

void HeaterPID::end() {
  DEBUG.println("HeaterPID::end called!");
  turnedOff = true;
  this->update();
}

ESP8266WebServer::THandlerFunction HeaterPID::getTriggerAutoTuneCallback() {
  return std::bind(&HeaterPID::triggerAutoTune, this);
}
