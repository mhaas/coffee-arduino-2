#include "HeaterPID.h"

HeaterPID::HeaterPID(SettingsStorage *_settings, MQTTLogger *_logger) {
  settings = _settings;
  logger = _logger;
  turnedOff = true;
  timesBetweenLoops = new Average<unsigned long>(1000);
}

void HeaterPID::begin(const int _relayPin) {
  relayPin = _relayPin;
  pinMode(relayPin, OUTPUT);
  disableHeater();

  // We could use P_ON_M as this should have a great benefit
  // for "integrative" applications, e.g. heating.
  // However, literature on tuning P_ON_M (or I-PD, as they are sometimes
  // called)
  // is really scarce and it makes the P part more complex (also keeping a
  // history on that),
  // so P_ON_E is easier to handle.
  //
  // See
  // http://brettbeauregard.com/blog/2017/06/introducing-proportional-on-measurement
  // for a details on P_ON_M
  // Caveat: Brettb states that the benefit is only realzed on setpoint
  // (e.g. desired temp) changes:
  // https://groups.google.com/d/msg/diy-pid-control/010XXPMMlfM/VZijD04WBAAJ
  // But in other messags in that thread, there seems to be a benefit
  // even with a steady setpoint (e.g. less overshoot)
  // All in all, I hope this allows more aggressive (faster) temperature
  // recovery
  // without as much overshot.

  // kP: unitless
  // kI: 1 * s
  // kD: 1/s
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

  // Only sample once every period. Otherwise, the errors will be accumulated
  // more than once per sample period (if we called compute() more often on the
  // PID),
  // but we ignore any changed PID output,
  // hence the error will grow larger.
  // This call should thus reduce erratic behavior by the PID algorithm.
  pid->SetSampleTime(HEATER_WINDOW_SIZE);

  // turn the PID on
  // TODO: initialize the temperature sensor variables
  pid->SetMode(AUTOMATIC);

  turnedOff = false;
}

void HeaterPID::update() {

  unsigned long currentLoopStartTime = millis();

  if (lastLoopStartTime == 0) {
    lastLoopStartTime = currentLoopStartTime;
  } else {
    unsigned int timeSinceLastLoop = currentLoopStartTime - lastLoopStartTime;
    lastLoopStartTime = currentLoopStartTime;
    timesBetweenLoops->push(timeSinceLastLoop);
  }

  if (!this->checkSanity()) {
    logger->println(
        "HeaterPID::update: failed sanity check. Disabling heater!");
    this->settings->flagInternalError();
    this->disableHeater();
    this->turnOffPIDAlgorithm();
    return;
  }

  if (this->turnedOff) {
    logger->println("HeaterPID::update: turnedOff is true! Disabling heater");
    this->disableHeater();
    this->turnOffPIDAlgorithm();
    return;
  }

  // Duration of one AC period. 20ms for 50Hz
  const double PERIOD_DURATION = 20.0;

  // How many milliseconds have passed since we have started the current window?
  float currentWindowElapsed = millis() - windowStartTime;

  if (currentWindowElapsed > HEATER_WINDOW_SIZE) {
    // logger->print("Starting new window.");
    // We may have gotten an update on the PID parameters, so update
    // them on every cycle
    // pid->SetTunings(settings->getKp(), settings->getKi(), settings->getKd());

    // Every time we start a new window, record statistics about time between
    // loop invocations
    this->settings->pidLoopMin = timesBetweenLoops->minimum();
    this->settings->pidLoopMax = timesBetweenLoops->maximum();
    this->settings->pidLoopAvg = timesBetweenLoops->mean();
    this->settings->pidLoopStdDev = timesBetweenLoops->stddev();
    //timesBetweenLoops->clear();

    // time to shift the Relay Window
    windowStartTime = millis();
    currentWindowElapsed = 0;
    // We only switch on once and off once during HEATER_WINDOW_SIZE.
    // For now, we only compute the PID output once during that period:
    // But we can compute it more often and then adjust the heating duration
    // as long as it does not cause additional switches
    pid->Compute();
    // logger->println(String("PID says: ") + String(pidOutput));
    // We always switch after a number of full periods. If we do not respect
    // the period, the SSR may turn off after an uneven number of half-periods,
    // which can lead to noise in the power grid.
    // See again the TAB on the FH stralsund server, although the
    // "Unsymmetrische Schwingungspaketsteuerungen" mentioned there may refer
    // to continously switching only one half-period
    // Also see https://de.wikipedia.org/wiki/Schwingungspaketsteuerung
    double roundedPidOutput =
        round(pidOutput / PERIOD_DURATION) * PERIOD_DURATION;
    settings->setPidOutput(roundedPidOutput);
    // TODO: for this logic to work, the function must be called every
    // millisecond
    // If we know an offset, we should subtract that here
    // logger->println(String("PID output rounded to grid frequency: ") +
    // String(roundedPidOutput));
    // logger->println(String("Desired Temp: ") +
    // String(*(settings->getDesiredTemperature())));
    // Publish internal PID variable for debugging!
    settings->setPidInternalOutputSum(pid->GetOutputSum());
    settings->setPidInternalPTerm(pid->getLastPTerm());
    settings->setPidInternalITerm(pid->getLastITerm());
    settings->setPidInternalDTerm(pid->getLastDTerm());
  }

  // Now decide if we are in the duty cycle of our period, i.e. if we want
  // to turn the relay for the header on or off
  if (currentWindowElapsed > pidOutput) {
    disableHeater();
  } else {
    enableHeater();
  }
}

void HeaterPID::enableHeater() { digitalWrite(relayPin, HEATER_RELAY_ON); }

void HeaterPID::disableHeater() { digitalWrite(relayPin, HEATER_RELAY_OFF); }

/**
* Performs some checks if observed and desired values look sane. Returns false
* if something bad is observed, e.g. temperature too high.
*/
boolean HeaterPID::checkSanity() {
  if (HEATER_WINDOW_SIZE < 600) {
    // See Table 2 here:
    // http://antriebstechnik.fh-stralsund.de/1024x768/Dokumentenframe/Kompendium/TAB/TAB.htm
    // For the limits
    // This table is not in the current TAB, it seems, but should still be
    // relevant
    logger->println("HeaterPID: HEATER_WINDOW_SIZE too small. Switching this "
                    "often puts noise on the power grid.");
    return false;
  }
  double currentTemperature = *settings->getCurrentTemperature();
  if (isnan(currentTemperature)) {
    logger->println("HeaterPID:checkSanity: observed temperature is NaN!");
    return false;
  }
  if (currentTemperature == TempSensor::INVALID_READING) {
    logger->println("HeaterPID: observed temperature is INVALID_READING!");
    return false;
  }
  if (currentTemperature > MAX_TEMPERATURE_ALLOWED) {
    logger->println(
        "HeaterPID: observed temperature higher than MAX_TEMPERATURE_ALLOWED");
    return false;
  }
  double desiredTemperature = *settings->getDesiredTemperature();
  if (isnan(desiredTemperature)) {
    logger->println("HeaterPID: desired temperature is NaN!");
    return false;
  }
  if (desiredTemperature > MAX_TEMPERATURE_ALLOWED) {
    logger->println(
        "HeaterPID: desired temperature higher than MAX_TEMPERATURE_ALLOWED");
    return false;
  }
  if (desiredTemperature < 0) {
    logger->println("HeaterPID: desired temperature < 0!");
    return false;
  }
  if (currentTemperature - 10 > desiredTemperature && pidOutput > 0) {
    // This can happen e.g. if kI is very big and contributes lots of overshoot.
    logger->println("HeaterPID: Overshooting temperature >= 10, but pidOutput "
                    "> 0. Disabling!");
    return false;
  }
  return true;
}

void HeaterPID::turnOffPIDAlgorithm() {
  // Disables the PID algorithm and sets the pidOutput to zero.
  // Does not prevent update() from running.
  logger->println("HeaterPID::turnOffPIDAlgorithm called!");
  pid->SetMode(MANUAL);
  pidOutput = 0;
}

void HeaterPID::end() {
  logger->println("HeaterPID::end called!");
  turnedOff = true;
  this->update();
}