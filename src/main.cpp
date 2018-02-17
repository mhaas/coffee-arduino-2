/**
 *
 * A http-enabled PID controller for espresso machines.
 *
 */

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <Ticker.h>

#include "WebServer.h"
#include "TempSensor.h"
#include "WifiWrapper.h"
#include "HeaterPID.h"
#include "MQTTPublisher.h"
#include "MQTTLogger.h"
#include "secret.h"

// The loop is not allowed to take more than 2s. That's a lot already, given
// that the pid cycle is only 600ms.
#define MAIN_LOOP_WATCHDOG_TIMEOUT 2000

/**
 * End configuration constants
 */

SettingsStorage settings = SettingsStorage();

MQTTPublisher publisher = MQTTPublisher(&settings);

MQTTLogger logger = MQTTLogger(&publisher);

WebServer httpd = WebServer(&settings);

TempSensor tempSensor = TempSensor(&settings, &logger);

WifiWrapper wifi = WifiWrapper();

HeaterPID pid = HeaterPID(&settings, &logger);

Ticker loopWatchDog;

void restart() {
  pid.end();
  logger.println("Watchdog: loopWatchDog restart!");
  ESP.restart();
}

// the setup function runs once when you press reset or power the board
void setup() {
  DEBUG.begin(DEBUG_BEGIN_ARG);
  settings.begin();
  // Initialize and reset pins
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
    logger.println("OTA started!");
    logger.println("Turning off HeaterPID");
    pid.end();
    logger.println("Disabling main loop watchdog!");
    loopWatchDog.detach();
  });
  ArduinoOTA.begin();

  publisher.begin(MQTT_HOST, 1883);
 }

 void rearmWatchDog() {
   loopWatchDog.detach();
   loopWatchDog.attach_ms(MAIN_LOOP_WATCHDOG_TIMEOUT, restart);
 }

// the loop function runs over and over again forever
void loop() {
  rearmWatchDog();
  ArduinoOTA.handle();
  httpd.update();
  wifi.update();
  tempSensor.update();
  pid.update();
  publisher.update();
  rearmWatchDog();
}
