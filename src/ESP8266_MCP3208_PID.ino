/**
 * 
 * A http-enabled PID controller for espresso machines.
 *
 */



#include "SettingsStorage.h"
#include "WebServer.h"
#include "TempSensor.h"
#include "WifiWrapper.h"
#include "HeaterPID.h"

#define DEBUG Serial



/**
 * Some configuration constants
 */
const char *SSID = "SSID";
const char *PASSWORD = "pass";
const byte LED_PIN = 0;
const byte HEATER_RELAY_PIN = 5;
const byte SPI_CS_PIN = 16;
const byte HTTPD_PORT = 80;
const float ANALOG_REFERENCE = 3.3;
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

WifiWrapper wifi = WifiWrapper(&DEBUG);

HeaterPID pid = HeaterPID(&settings, &DEBUG);

// the setup function runs once when you press reset or power the board
void setup() {
  // Initialize Serial with 9600 baud because my USB<->Serial can't handle more
  DEBUG.begin(9600);

  // TODO: are the relays low-active?
  // https://arduino-info.wikispaces.com/ArduinoPower
  // Initialize and reset pins
  digitalWrite(LED_PIN, LOW);
  pinMode(LED_PIN, OUTPUT);

  settings.begin();
  // Connect URI-based triggers
  // While global settings are handled via the SettingsStorage registry, simple void functions are
  // connected to the web server via callbacks
  httpd.addTrigger("/trigger_autotune", std::bind(&HeaterPID::triggerAutoTune, pid));
  httpd.begin();
  wifi.begin(SSID, PASSWORD);
  pid.begin(HEATER_RELAY_PIN);
  tempSensor.begin(SPI_CS_PIN, ANALOG_REFERENCE);
}

// the loop function runs over and over again forever
void loop() {
  httpd.update();
  tempSensor.update();
  wifi.update();
  pid.update();
}

