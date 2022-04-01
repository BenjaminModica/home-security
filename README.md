# Home_Security
Home security project using an ESP8266 as a web server and controller for sensors.
 
Source for libraries: https://github.com/esp8266/Arduino.

Demo-video (I am holding a magnet that represents the door opening and closing)

https://youtu.be/7-OJu9-Q4V0

A note on versions: 

V1 is scratched

V2 uses an external website hosted on firebase as control panel. This required a secure https connection to esp8266 which made it slow, esp is not good for SSL cryptography. 

V3 hosts the webpage on esp server. A lot easier to work with than V2. 
