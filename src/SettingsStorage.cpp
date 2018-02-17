#include "SettingsStorage.h"

// The config version is checked to see if a (current) configuration
// is available.

// Code adapted from here:
// http://playground.arduino.cc/Code/EEPROMLoadAndSaveSettings

void SettingsStorage::begin() {

  EEPROM.begin(sizeof(storage));
  // Read config from EEPROM
  EEPROM.get(CONFIG_DATA_OFFSET, this->storage);

  if (strncmp(this->storage.configVersion, CONFIG_VERSION,
              CONFIG_VERSION_LENGTH) != 0) {
    // If you don't see this message, add a delay() to make sure the serial port
    // is initialized
    delay(100);

    DEBUG.println("SettingsStorage::begin: no settings found in EEPROM. "
                  "Storing defaults!");
    // no config found  - write default config from header file into EEPROM
    EEPROM.put(CONFIG_DATA_OFFSET, this->defaultSettings);
    EEPROM.commit();
  }
  // Load config from EEPROM
  EEPROM.get(CONFIG_DATA_OFFSET, this->storage);
}

void SettingsStorage::update() {
  // no-op
}

void SettingsStorage::store(double value, double *target) {
  *target = value;
  EEPROM.put(CONFIG_DATA_OFFSET, this->storage);
  EEPROM.commit();
}

void SettingsStorage::setKp(double kp) { store(kp, &this->storage.kp); }

void SettingsStorage::setKi(double ki) { store(ki, &this->storage.ki); }

void SettingsStorage::setKd(double kd) { store(kd, &this->storage.kd); }

void SettingsStorage::setDesiredTemperature(double desiredTemperature) {
  store(desiredTemperature, &this->storage.desiredTemperature);
}

void SettingsStorage::setCurrentTemperature(double _currentTemperature) {
  currentTemperature = _currentTemperature;
}

void SettingsStorage::setPidOutput(double _pidOutput) {
  pidOutput = _pidOutput;
}

void SettingsStorage::setPidInternalOutputSum(double _pidInternalOutputSum) {
  pidInternalOutputSum = _pidInternalOutputSum;
}

void SettingsStorage::setPidInternalPTerm(double _pidInternalPTerm) {
  pidInternalPTerm = _pidInternalPTerm;
}

void SettingsStorage::setPidInternalITerm(double _pidInternalITerm) {
  pidInternalITerm = _pidInternalITerm;
}

void SettingsStorage::setPidInternalDTerm(double _pidInternalDTerm) {
  pidInternalDTerm = _pidInternalDTerm;
}

void SettingsStorage::setTempOffset(double tempOffset) {
  store(tempOffset, &this->storage.tempOffset);
}

void SettingsStorage::flagInternalError() {
  internalError = true;
}

double *SettingsStorage::getDesiredTemperature() {
  return &this->storage.desiredTemperature;
}

double *SettingsStorage::getCurrentTemperature() {
  return &this->currentTemperature;
}

double SettingsStorage::getPidOutput() { return pidOutput; }

double SettingsStorage::getTempOffset() { return this->storage.tempOffset; }

double SettingsStorage::getKp() { return this->storage.kp; }

double SettingsStorage::getKi() { return this->storage.ki; }

double SettingsStorage::getKd() { return this->storage.kd; }

void SettingsStorage::toJSON(String &dest) {
  const int BUFFER_SIZE = JSON_OBJECT_SIZE(14);
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

  JsonObject &object = jsonBuffer.createObject();

  object["current_temperature"] = *(this->getCurrentTemperature());
  object["desired_temperature"] = *(this->getDesiredTemperature());
  object["temp_offset"] = this->getTempOffset();
  object["kp"] = this->getKp();
  object["ki"] = this->getKi();
  object["kd"] = this->getKd();
  object["pid_output"] = this->getPidOutput();
  object["free_heap"] = ESP.getFreeHeap();
  object["uptime_milliseconds"] = millis();
  object["pid_internal_output_sum"] = pidInternalOutputSum;
  object["pid_internal_p_term"] = pidInternalPTerm;
  object["pid_internal_i_term"] = pidInternalITerm;
  object["pid_internal_d_term"] = pidInternalDTerm;
  object["internal_error"] = internalError;

  // see http://blog.benoitblanchon.fr/arduino-json-v5-0/
  // for a discussion on static VS dynamic allocation on embedded platforms
  object.printTo(dest);
}
