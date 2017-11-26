#include "WifiWrapper.h"

WifiWrapper::WifiWrapper(Stream* _dbgStream) {
  dbgStream = _dbgStream;
}

void WifiWrapper::begin(const char* ssid, const char* psk) {
  WiFi.begin(ssid, psk);
}

void WifiWrapper::update() {

  int now = millis();
  if (now > wifiLastCheck + WIFI_UPDATE_INTERVALL) {
    wifiLastCheck = now;
    wl_status_t currentState = WiFi.status();
    // see https://github.com/sandeepmistry/esp8266-Arduino/blob/master/esp8266com/esp8266/libraries/ESP8266WiFi/src/include/wl_definitions.h
    // for valid status flags
    if (currentState != WL_CONNECTED) {
      dbgStream->print(".");
    } else if (currentState == WL_CONNECTED && wifiLastState != WL_CONNECTED) {
      dbgStream->println("");
      dbgStream->print("Connected to ");
      dbgStream->println(WiFi.SSID());
      dbgStream->print("IP address: ");
      dbgStream->println(WiFi.localIP());  
    }
    wifiLastState = currentState;
  }



}
