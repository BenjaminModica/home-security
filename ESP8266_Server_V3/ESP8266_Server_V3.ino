#include <arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP_Mail_Client.h>
#include "secrets.h" //add WLAN credentials in here
#include "webpage.h"

//Host and port for gmail adress
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

//SMTP session used for sending emails
SMTPSession smtp;

/**
 * Send e-mail when alarm is triggered
 */
void sendMessage() {
  //Alarm triggered email setup
  smtp.debug(1);
  smtp.callback(smtpCallback);
  
  ESP_Mail_Session session;
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";
  SMTP_Message message;
  
  message.sender.name = "ESP";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "ALARM TRIGGERED";
  message.addRecipient("Benjamin", RECIPIENT_EMAIL);
  String htmlMsg = "<div style=\"color:#2f4468;\"><h1>ALARM TRIGGERED</h1><p>- Sent from ESP board</p></div>";
  message.html.content = htmlMsg.c_str();
  message.html.content = htmlMsg.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  if (!smtp.connect(&session))
    return;

  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("Error sending Email, " + smtp.errorReason());
  }
}

/**
 * Debug smtp
 */
void smtpCallback(SMTP_Status status) {
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()) {
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++) {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients);
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}

const char *dname = "homecontrolpanel";

ESP8266WebServer server(8000);

const int readyLed = 13;
const int onLed1 = 0;
const int onLed2 = 12;
//const int button = 15; Has pulldown resistor, use as button
const int trigPin = 5;
const int echoPin = 4;
const int pirSensor = 16;
boolean doorOpen = false;
const int blueLed = 2;
int timer = millis();
const int period = 2000;

//STATES
//alarmOff: Alarm OFF
//alarmOn: Alarm READY
//alarmTrig: Alarm ON
String state;

void handleRoot() {
  server.send(200, "text/html", webpage);
}

/**
   Handles GET requests for /alarm/.
   Data recieved have to be alarmOff or alarmOn
   Sends state as response
*/
void handleAlarm() {
  //server.send(200, "text/plain", "Change alarm: /alarm/data?=<alarmOff or alarmOn>");
  String data = server.arg("data");
  Serial.println("Get request for handleAlarm");
  Serial.print("data recieved is:   ");
  Serial.println(data);

  if (data == "alarmOff") {
    state = "alarmOff";
    server.send(200, "text/plain", String(state));
  } else if (data == "alarmOn") {
    state = "alarmOn";
    server.send(200, "text/plain", String(state));
  } else {
    Serial.println("Cannot handle that request");
    server.send(200, "text/plain", "Cannot handle that request");
  }
}

/**
   Handles get requests for /status/
   Sends the state as response
*/
void handleStatusRequest() {
  server.send(200, "text/plain", String(state));
}

void setup(void) {
  //pin and serial setup
  pinMode(readyLed, OUTPUT);
  pinMode(onLed1, OUTPUT);
  pinMode(onLed2, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  //pinMode(button, INPUT);
  digitalWrite(readyLed, 0);
  digitalWrite(onLed1, 0);
  digitalWrite(onLed2, 0);
  Serial.begin(115200);
  pinMode(blueLed, OUTPUT);
  digitalWrite(blueLed, 0);
  pinMode(pirSensor, INPUT);

  //wifi setup
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, passWord);
  Serial.println("");

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
  digitalWrite(blueLed, 1);

  state = "alarmOff";
}

void loop(void) {
  server.handleClient();
  MDNS.update();
  updateState();
  
  if (state == "alarmTrig") {
    loopLEDs();
  }
  
  if (state == "alarmOn" && millis() >= timer + period) {
    distanceSensor();
    timer = millis();
  }
  
  //Will happen once when alarm is triggered
  if (digitalRead(pirSensor) == HIGH && state == "alarmOn" && doorOpen) {
    state = "alarmTrig";
    Serial.println("detects stuff");
    loopLEDs();
    sendMessage(); //Sends one email when alarm is triggered
  }
}

/**
   Updates pins depending on state
*/
void updateState() {
  if (state == "alarmOff") { //Alarm is off, no led's on
    digitalWrite(readyLed, 0);
    digitalWrite(onLed1, 0);
    digitalWrite(onLed2, 0);
  } else if (state == "alarmOn") { //alarm is set, green led on
    digitalWrite(readyLed, 1);
    digitalWrite(onLed1, 0);
    digitalWrite(onLed2, 0);
  } else if (state == "alarmTrig") { //alarm triggered: led's handled by loopLEDs()
    return;
  } else {
    Serial.println("state outside of range");
  }
}

/*
   Flashes two LEDs if alarm is triggered
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

/**
 * Takes care of everything to do with the distance sensor
 */
void distanceSensor() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  const unsigned long duration= pulseIn(echoPin, HIGH);
  int distance= duration/29/2;

  if(duration==0){
   Serial.println("Warning: no pulse from sensor");
  } 
  else{
      //For debugging purposes:
      Serial.print("distance to nearest object: ");
      Serial.print(distance);
      Serial.println(" cm");
  }
  delay(100);

  if (distance < 10) { 
    doorOpen = true;
  } else {
    doorOpen = false;
  }
}
