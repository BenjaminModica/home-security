# Home Security
IoT home security project using a Raspberry Pi Pico as a TCP server and controller for PIR sensor and RFID tag reader.

~~Home security project using an ESP8266 as a web server and controller for sensors.~~

## Versions 

### pico 
Using a Pi Pico as the controller.

### V3
Hosts the webpage on esp server. A lot easier to work with than V2. Source for libraries: https://github.com/esp8266/Arduino. Demo-video (I am holding a magnet that represents the door opening and closing) https://youtu.be/7-OJu9-Q4V0.

### V2 
Uses an external website hosted on firebase as control panel. This required a secure https connection to esp8266 which made it too slow, esp is not good for SSL cryptography. 

### V1 
Removed.