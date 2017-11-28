/**
 *
 * A http-enabled PID controller for espresso machines.
 *
 */


#include <Arduino.h>
#include "SettingsStorage.h"
#include "WebServer.h"
#include "TempSensor.h"
#include "WifiWrapper.h"
#include "HeaterPID.h"
#include "secret.h"

#define DEBUG Serial



/**
 * Default desired heater temperature in degrees celsius if no
 * value is set in the EEPROM
 */


/**
 * End configuration constants
 */

SettingsStorage settings = SettingsStorage();

WebServer httpd = WebServer(&settings);

TempSensor tempSensor = TempSensor(&settings);

WifiWrapper wifi = WifiWrapper();

HeaterPID pid = HeaterPID(&settings);

// the setup function runs once when you press reset or power the board
void setup() {
  // Initialize Serial with 9600 baud because my USB<->Serial can't handle more
  DEBUG.begin(9600);

  // TODO: are the relays low-active?
  // https://arduino-info.wikispaces.com/ArduinoPower
  // Initialize and reset pins

  settings.begin();
  // Connect URI-based triggers
  // While global settings are handled via the SettingsStorage registry, simple void functions are
  // connected to the web server via callbacks
  httpd.addTrigger("/trigger_autotune", std::bind(&HeaterPID::triggerAutoTune, pid));
  httpd.begin();
  wifi.begin(SSID, PASSWORD);
  pid.begin(HEATER_RELAY_PIN);
  tempSensor.begin(TEMP_SENSOR_CS_PIN);
}

// the loop function runs over and over again forever
void loop() {
  httpd.update();
  tempSensor.update();
  wifi.update();
  pid.update();
}
