#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WebServerSecure.h>
#include <ESP8266mDNS.h>
#include "secrets.h" //add WLAN credentials in here
#include "certificate.h"

//#ifndef STASSID
//#define STASSID "SSID"
//#define STAPSK  "Password"
//#endif

//const char* ssid = STASSID;
//const char* password = STAPSK;
const char *dname = "homecontrolpanel";

//void handleNotFound() {}

BearSSL::ESP8266WebServerSecure server(443);
//ESP8266WebServer serverHTTP(8080);

const int readyLed = 13;
const int onLed1 = 0;
const int onLed2 = 12;
//const int button = 15; Has pulldown resistor, great for using as button!
//const int trigPin = 5;
//const int echoPin = 4;
const int pirSensor = 16;


//STATES
//0: Alarm OFF
//1: Alarm READY
//2: Alarm ON
int state = 0;    

//void secureRedirect() {
//  serverHTTP.sendHeader("Location", String("https://homecontrolpanel.local"), true);
//  serverHTTP.send(301, "text/plain", "");
//}

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
  pinMode(onLed1, OUTPUT);
  pinMode(onLed2, OUTPUT);
  //pinMode(button, INPUT);
  //pinMode(trigPin, OUTPUT); 
  //pinMode(echoPin, INPUT); //Don't forget to input 3.3V and not 5V from motion sensor!
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

  if (MDNS.begin("dname")) {
    Serial.println("MDNS responder started");
    Serial.print("Server can be accessed at https://");
    Serial.println(WiFi.localIP());
//    Serial.print(" or at https://");
//    Serial.print(dname);
//    Serial.println(".local");
  }

  configTime(1 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  //configTime("CET-1CEST,M3.5.0,M10.5.0/3" , "pool.ntp.org", "time.nis.gov");
  // Wait till time is synced
  Serial.print("Syncing time");
  int i = 0;
  while (time(nullptr) < 1000000000ul && i<100) {
    Serial.print(".");
    delay(100);
    i++;
  }
  Serial.println();

  time_t tnow = time(nullptr);
  struct tm *timeinfo;
  char buffer [80];


  timeinfo = localtime (&tnow);
  strftime (buffer,80,"Local time: %H:%M.",timeinfo);
  Serial.println(buffer);
  
  timeinfo = gmtime (&tnow);
  strftime (buffer,80,"UTC time: %H:%M.",timeinfo);
  Serial.println(buffer); 

//  serverHTTP.on("/", secureRedirect);
//  serverHTTP.begin();
//  Serial.println("HTTP server started");

  server.getServer().setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));

  server.on("/", handleRoot);

  server.on("/alarm", handleAlarm);

  server.on("/status", handleStatusRequest);

  //server.onNotFound(handleNotFound);

//  serverHTTP.enableCORS(true);
  server.enableCORS(true);
  //server.setInsecure();
  server.begin();
  Serial.println("HTTPS server started");

  state = 0;
}

void loop(void) {
//  serverHTTP.handleClient();
  server.handleClient();
  MDNS.update();
  updateState();
  //distanceSensor();
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
