#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "secrets.h" //add WLAN credentials in here

//#ifndef STASSID
//#define STASSID "SSID"
//#define STAPSK  "Password"
//#endif

//const char* ssid = STASSID;
//const char* password = STAPSK;

//void handleNotFound() {}

ESP8266WebServer server(80);

const int readyLed = 13;
const int onLed = 15;
const int button = 2;

//STATES
//0: Alarm OFF
//1: Alarm READY
//2: Alarm ON
int state = 0;    

void handleRoot() {
  server.send(200, "text/plain", "hello\r\n");
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
  server.send(200, "text/plain", String(state));
}

void setup(void) {
  //pin and serial setup
  pinMode(readyLed, OUTPUT);
  pinMode(onLed, OUTPUT);
  pinMode(button, INPUT);
  digitalWrite(readyLed, 0);
  digitalWrite(onLed, 0);
  Serial.begin(115200);
  //pinMode(LED_BUILTIN, OUTPUT);
  //digitalWrite(LED_BUILTIN, 1);

  //wifi setup
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, passWord);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("homecontrolpanel")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/alarm", handleAlarm);

  server.on("/status", handleStatusRequest);

  //server.onNotFound(handleNotFound);

  server.enableCORS(true);
  server.begin();
  Serial.println("HTTP server started");

  state = 0;
}

void loop(void) {
  server.handleClient();
  MDNS.update();
  updateState();
  if (digitalRead(button) == 0 && state == 1) {state = 2; Serial.println("Alarm tripped");} //Only trip alarm if set to ready
}

/**
 * Updates pins depending on state
 */
void updateState() {
  switch (state) { 
    case 0: //Alarm off
      digitalWrite(readyLed, 0);
      digitalWrite(onLed, 0);
      break;
    case 1: //Alarm ready
      digitalWrite(readyLed, 1);
      digitalWrite(onLed, 0);
      break;
    case 2: //Alarm on
      digitalWrite(onLed, 1);
      break;
    default:
      break;
  }
}
