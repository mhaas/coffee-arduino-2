#include "HeaterPID.h"
// To get INVALID_READING
#include "TempSensor.h"

HeaterPID::HeaterPID(SettingsStorage *_settings) {
  settings = _settings;
  autoTuneRequested = false;
}

void HeaterPID::begin(const int _relayPin) {
  // TODO: limit desiredTemperature?
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

  if (!this->checkSanity()) {
    DEBUG.println("HeaterPID: failed sanity check. Disabling heater!");
    this->disableHeater();
    return;
  }

  pid->Compute();
  settings->setPidOutput(pidOutput);

  // How many milliseconds have passed since we have started the current window?
  float currentWindowElapsed = millis() - windowStartTime;

  if (currentWindowElapsed > HEATER_WINDOW_SIZE) {
    // time to shift the Relay Window
    // TODO: just use current timestamp here?
    windowStartTime += HEATER_WINDOW_SIZE;
    DEBUG.print("Starting new window. PID says: ");
    DEBUG.println(pidOutput);
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
  aTune = new PID_ATune(settings->getCurrentTemperature(),
                        settings->getDesiredTemperature());
  // 0: PI, 1: PID
  aTune->SetControlType(0);
  // TODO: there are parameters which look interesting, like
  // SetNoiseBand and SetLookbackSec. The defaults look useful.
  autoTuneRequested = true;
}

/**
* Performs some checks if observed and desired values look sane. Returns false
* if something bad is observed, e.g. temperature too high.
*/
boolean HeaterPID::checkSanity() {
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
