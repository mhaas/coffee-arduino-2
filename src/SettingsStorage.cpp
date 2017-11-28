#include "SettingsStorage.h"
#include <EEPROM.h>


// The config version is checked to see if a (current) configuration
// is available.

// Code adapted from here: http://playground.arduino.cc/Code/EEPROMLoadAndSaveSettings

// TODO: handle global variables
// TODO: perhaps store config in PidSetting struct?



void SettingsStorage::begin() {
  EEPROM.begin(sizeof(storage));
  // Read config from EEPROM
  EEPROM.get(CONFIG_DATA_OFFSET, this->storage);
  if (strncmp(this->storage.configVersion, CONFIG_VERSION, CONFIG_VERSION_LENGTH) != 0) {
    DEBUG.println("SettingsStorage::begin: no settings found in EEPROM. Storing defaults!");
    // no config found  - write default config from header file into EEPROM
    EEPROM.put(CONFIG_DATA_OFFSET, this->storage);
    EEPROM.commit();
  }
  // Load config from EEPROM
  EEPROM.get(CONFIG_DATA_OFFSET, this->storage);
}

void SettingsStorage::update() {

}

void SettingsStorage::store(double value, double* target) {
  *target = value;
  EEPROM.put(CONFIG_DATA_OFFSET, this->storage);
  EEPROM.commit();
}


void SettingsStorage::setKp(double kp) {
  store(kp, &this->storage.kp);
}

void SettingsStorage::setKi(double ki) {
  store(ki, &this->storage.ki);
}

void SettingsStorage::setKd(double kd) {
  store(kd, &this->storage.kd);
}

void SettingsStorage::setDesiredTemperature(double desiredTemperature) {
  store(desiredTemperature, &this->storage.desiredTemperature);
}

void SettingsStorage::setCurrentTemperature(double _currentTemperature) {
  currentTemperature = _currentTemperature;
}

void SettingsStorage::setTempOffset(double tempOffset) {
  store(tempOffset, &this->storage.tempOffset);
}

double* SettingsStorage::getDesiredTemperature() {
  return &this->storage.desiredTemperature;
}

double* SettingsStorage::getCurrentTemperature() {
  return &this->currentTemperature;
}

double SettingsStorage::getTempOffset() {
  return this->storage.tempOffset;
}

double SettingsStorage::getKp() {
  return this->storage.kp;
}

double SettingsStorage::getKi() {
  return this->storage.ki;
}

double SettingsStorage::getKd() {
  return this->storage.kd;
}
