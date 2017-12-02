#include "WifiWrapper.h"


void WifiWrapper::begin(const char* ssid, const char* psk) {
  WiFi.hostname("espresso");
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
      DEBUG.print(".");
    } else if (currentState == WL_CONNECTED && wifiLastState != WL_CONNECTED) {
      DEBUG.println("");
      DEBUG.print("Connected to ");
      DEBUG.println(WiFi.SSID());
      DEBUG.print("IP address: ");
      DEBUG.println(WiFi.localIP());
    }
    wifiLastState = currentState;
  }



}
