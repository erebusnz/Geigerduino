
Before you start:

* If you have regular 5v Arduino, see '**Geigerduino**'
* If you have ESP32 Arduino, see '**Geigerduino ESP32**' (supporting serial only) or '**Geigerduino ESP32 AWS IOT**' (supporting serial and AWS IOT thing over WiFi)

# Geigerduino (Arduino 5v)

Geigerduino Serial Geiger–Müller Counter

*Compatible with RadiationD-v1.1-CAJOE, 52DZ-DIY.com marked boards and many more with 5v trigger output*

Conversion factor is for J305 marked tube. Update to match your tube. See https://www.cooking-hacks.com/documentation/tutorials/geiger-counter-radiation-sensor-board-arduino-raspberry-pi-tutorial#cpm_to_servants

### Instructions:

1. Confirm you have 5v logic-level Arduino! Otherwise you will blow up a 3.3v interupt pin or the Arduino itself. These cheap boards will output 5v trigger.
1. Connect Gnd to Gegier board and Arudino
1. Connect 5v power to Geiger board and Arduino 5v pin
1. Connect Geiger counter pin to Arudino, INT0, either pin 2 or 3 (see https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/)
1. Connect USB and monitor Serial port, 9600 BAUD (you can use ArduinoIDE inbuilt Serial Monitor, or something like PuTTY)
1. Use Geigerduino.ino

# Geigerduino ESP32

*Compatible with RHelectronics.com marked boards and many more which have pull-to-ground interrupt output requiring internal Aruduino pullup resister*

Conversion factor is for CTC-5 (STS-5) marked tube only. Update to match your tube. See https://www.cooking-hacks.com/documentation/tutorials/geiger-counter-radiation-sensor-board-arduino-raspberry-pi-tutorial#cpm_to_servants

### Instructions:

1. Make sure your board doesn't use 5v interrupt! Use a Oscilliscope to verify it doesn't output 5v (or check documentation). This method is designed to use a 'pull down to ground' interrupt PIN which relies on using Aruidno to pull up.
1. Connect Gnd to Gegier board and Arudino (these boards have 3pin output for this)
1. Connect 5v power to Geiger board and Arduino (these boards have 3pin output for this)
1. Connect Geiger counter pin to Arudino on an 'interrupt' compatible PIN (see https://randomnerdtutorials.com/esp32-pinout-reference-gpios/ and choose a 'GREEN' pin for input)
1. Connect USB and monitor Serial port, 9600 BAUD (you can use ArduinoIDE inbuilt Serial Monitor, or something like PuTTY)
1. Use GeigerduinoESP32.ino

# Geigerduino ESP32 AWS IOT

(see above **Geigerduino ESP32**)

1. Follow instructions here https://www.instructables.com/id/Secure-IOT-With-AWS-and-Hornbill-ESP32-Using-Ardui/ to set up AWS IOT thing and obtain certificates
1. Edit the AWS IOT library 'aws_iot_certificate.c' file to add CA, private key and certificate data
1. Edit wifi_aws_credentials.h to match your WiFi and AWS IOT configuration
1. Use GeigerduinoESP32AWSIOT.ino
