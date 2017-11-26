#include "TempSensor.h"

TempSensor::TempSensor(SettingsStorage* _settings) {
  settings = _settings;
}

void TempSensor::begin(const byte _spiCsPin, const double _analogReference) {

  //sensor = new HighTempMCP320x(_spiCsPin, 0, 1);
  /* Some ADCs use 3.3.V, others use 5V as analog reference */
  //sensor->setAnalogReference(_analogReference);
  //sensor->begin();

}

void TempSensor::update() {

  // Sample every TEMP_INTERVALL ms
  int now = millis();
  if (now > lastRead + TEMP_INTERVALL) {
    lastRead = now;
    // Have to call getRoomTmp first..
    //sensor->getRoomTmp();

    settings->setCurrentTemperature(-1);
  }

}
