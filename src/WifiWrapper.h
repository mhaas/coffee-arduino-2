#ifndef WIFI_WRAPPER_H

#define WIFI_WRAPPER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

class WifiWrapper {

  public:
    WifiWrapper(Stream* _dbgStream);
    //~WifiWrapper();
    void begin(const char* ssid, const char* psk);
    void update();

  private:
    int wifiLastCheck = 0;
    wl_status_t wifiLastState = (wl_status_t) 0;
    Stream* dbgStream;
    const int WIFI_UPDATE_INTERVALL = 500;
};

#endif

