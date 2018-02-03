/**
 *
 * A http-enabled PID controller for espresso machines.
 *
 */


// TODO:  add own watchdog code to prevent heater from running too long:
// https://github.com/esp8266/Arduino/issues/1532


#include <Arduino.h>
#include <ArduinoOTA.h>
#include "WebServer.h"
#include "TempSensor.h"
#include "WifiWrapper.h"
#include "HeaterPID.h"
#include "MQTTPublisher.h"

#include "secret.h"




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

MQTTPublisher publisher = MQTTPublisher(&settings);

void restart() {
  ESP.restart();
}

// the setup function runs once when you press reset or power the board
void setup() {
  DEBUG.begin(DEBUG_BEGIN_ARG);
  // Initialize and reset pins
  settings.begin();
  pid.begin(HEATER_RELAY_PIN);
  // Connect URI-based triggers
  // While global settings are handled via the SettingsStorage registry, simple void functions are
  // connected to the web server via callbacks
  httpd.addTrigger("/trigger_autotune", pid.getTriggerAutoTuneCallback());
  httpd.addTrigger("/trigger_restart", restart);

  httpd.begin();
  wifi.begin(SSID, PASSWORD);
  tempSensor.begin(TEMP_SENSOR_CS_PIN);

  ArduinoOTA.onStart([]() {
    DEBUG.println("OTA started!");
    DEBUG.println("Turning off HeaterPID");
    pid.end();
  });
  ArduinoOTA.begin();

  publisher.begin(MQTT_HOST, 1883);

 }


// the loop function runs over and over again forever
void loop() {
  ArduinoOTA.handle();
  httpd.update();
  tempSensor.update();
  wifi.update();
  pid.update();
  publisher.update();
}
