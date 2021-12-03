#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "secrets.h" //add WLAN credentials in here
#include "webpage.h"

const char *dname = "homecontrolpanel";

ESP8266WebServer server(8000);

const int readyLed = 13;
const int onLed1 = 0;
const int onLed2 = 12;
//const int button = 15; Has pulldown resistor, use as button
//const int trigPin = 5;
//const int echoPin = 4;
const int pirSensor = 16;

//STATES
//0: Alarm OFF
//1: Alarm READY
//2: Alarm ON
int state = 0;    //Change this to a more descriptive name. ex: string state...

void handleRoot() {
  server.send(200, "text/html", webpage);
}


/**
 * Handles GET requests for /alarm/.
 * Data recieved have to be 0 or 1. 
 * Sends state as response
 */
void handleAlarm() {
  //server.send(200, "text/plain", "Change alarm: /alarm/data?=<0 or 1>");
  String data = server.arg("data");
  Serial.println("Get request for handleAlarm");
  Serial.print("data recieved is:   ");
  Serial.println(data);

  if (data == "0") {
    state = 0;
    server.send(200, "text/plain", String(state));
  } else if (data == "1") {
    state = 1;
    server.send(200, "text/plain", String(state));
  } else {
    Serial.println("Cannot handle that request");
    server.send(200, "text/plain", "Cannot handle that request");
  }
}

/**
 * Handles get requests for /status/
 * Sends the state as response
 */
void handleStatusRequest() {
//  server.send(200, "text/plain", String(state));
}

void setup(void) {
  //pin and serial setup
  pinMode(readyLed, OUTPUT);
  pinMode(onLed1, OUTPUT);
  pinMode(onLed2, OUTPUT);
  //pinMode(button, INPUT);
  digitalWrite(readyLed, 0);
  digitalWrite(onLed1, 0);
  digitalWrite(onLed2, 0);
  Serial.begin(115200);
  //pinMode(LED_BUILTIN, OUTPUT);
  //digitalWrite(LED_BUILTIN, 1);
  pinMode(pirSensor, INPUT);

  //wifi setup
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, passWord);
  Serial.println("");

  // Initialize SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("Error mounting SPIFFS");
    return;
  }

  // Wait for connection to wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("dname")) {
    Serial.println("MDNS responder started");
    Serial.print("Server can be accessed at http://");
    Serial.print(WiFi.localIP());
    Serial.println(":8000");
  }
  
  server.on("/", handleRoot);

  server.on("/alarm", handleAlarm);

  server.on("/status", handleStatusRequest);
  
  server.enableCORS(true);
  server.begin();
  Serial.println("HTTP server started");

  state = 0;
}

void loop(void) {
  server.handleClient();
  MDNS.update();
  updateState();
  if (state == 2) {loopLEDs();}

  if (digitalRead(pirSensor) == HIGH && state == 1) {
    state = 2;
    Serial.println("detects stuff");
  }
}

/**
 * Updates pins depending on state
 */
void updateState() {
  switch (state) { 
    case 0: //Alarm off
      digitalWrite(readyLed, 0);
      digitalWrite(onLed1, 0);
      digitalWrite(onLed2, 0);
      break;
    case 1: //Alarm ready
      digitalWrite(readyLed, 1);
      digitalWrite(onLed1, 0);
      digitalWrite(onLed2, 0);
      break;
    case 2: //Alarm on
      break;
    default:
      break;
  }
}

/*
 * Flashes two LEDs if alarm is tripped
 */
void loopLEDs() {
  if (!digitalRead(onLed1)) {
    digitalWrite(onLed1, HIGH);
    digitalWrite(onLed2, LOW);
  } else {
    digitalWrite(onLed1, LOW);
    digitalWrite(onLed2, HIGH);
  }
  delay(100);
}
