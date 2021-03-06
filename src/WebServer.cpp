
#include "WebServer.h"
#include <Arduino.h>
#include <FS.h>

WebServer::WebServer(SettingsStorage *_settings) {
  settings = _settings;
  httpd = new ESP8266WebServer(HTTPD_PORT);
}

/**
   Initialize webserver.
*/

void WebServer::begin() {

  SPIFFS.begin();
  /* Set up routing for the web server */
  // std::bind wraps a non-static method call so we can pass it as
  // a regular function pointer
  // See e.g.
  // http://arduino.stackexchange.com/questions/14157/passing-class-member-function-as-argument
  httpd->serveStatic("/", SPIFFS, "/wwwroot/index.html");
  httpd->serveStatic("/smoothie.js", SPIFFS, "/wwwroot/smoothie.js");
  httpd->on("/set", HTTP_POST, std::bind(&WebServer::handleSet, this));
  httpd->on("/get", HTTP_GET, std::bind(&WebServer::handleGet, this));
  httpd->onNotFound(std::bind(&WebServer::handleNotFound, this));
  httpd->begin();
}

void WebServer::handleNotFound() {
  httpd->send(404, "text/plain", "Not found");
}

void WebServer::handleSet() {
  String key;
  String value;
  String msg = "";
  bool fail = false;

  // Iterate over all keys
  // TODO: handle remaining keys
  for (int i = 0; i < httpd->args(); i++) {

    key = httpd->argName(i);
    value = httpd->arg(i);

    bool current_fail = false;

    if (key == DESIRED_TEMPERATURE_KEY) {
      settings->setDesiredTemperature(value.toFloat());
    } else if (key == TEMP_OFFSET_KEY) {
      settings->setTempOffset(value.toFloat());
    } else if (key == KP_KEY) {
      settings->setKp(value.toFloat());
    } else if (key == KI_KEY) {
      settings->setKi(value.toFloat());
    } else if (key == KD_KEY) {
      settings->setKd(value.toFloat());
    } else {
      current_fail = true;
    }

    if (!current_fail) {
      msg += "Processed key: ";
      msg += key;
      msg += ", value:";
      msg += value;
      msg += "\n";
    } else {
      msg += "Invalid key: ";
      msg += key;
    }

    fail = current_fail || fail;
  }

  if (fail) {
    httpd->send(500, "text/plain", msg);
  } else {
    httpd->send(200, "text/plain", msg);
  }
}

void WebServer::handleGet() {
  String json;
  settings->toJSON(json);
  httpd->send(200, "application/json", json);
}

void WebServer::update() { httpd->handleClient(); }

void WebServer::handleTrigger(ESP8266WebServer::THandlerFunction trigger) {
  // Small wrapper around a trigger which calls httpd->send() to end the
  // connection
  // From my reading of the source code, this does not happen automatically
  httpd->send(202, "text/plain", "Accepted");
  httpd->close();
  delay(500);
  trigger();
}

// Execute a callback if the given uri is requested via POST
void WebServer::addTrigger(const char *uri,
                           ESP8266WebServer::THandlerFunction trigger) {
  httpd->on(uri, HTTP_POST,
            std::bind(&WebServer::handleTrigger, this, trigger));
}
