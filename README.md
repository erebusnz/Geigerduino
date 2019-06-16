# Geigerduino

Geigerduino Serial Geiger–Müller Counter

*Compatible with RadiationD-v1.1-CAJOE, 52DZ-DIY.com marked boards and many more with 5v trigger output*

Conversion factor is for J305 marked tube only. Update to match your tube. See https://www.cooking-hacks.com/documentation/tutorials/geiger-counter-radiation-sensor-board-arduino-raspberry-pi-tutorial#cpm_to_servants

### Instructions:

1. Confirm you have 5v logic-level Arduino! Otherwise you will blow up a 3.3v interupt pin or the Arduino itself.
1. Connect Gnd to Gegier board and Arudino (these boards have 3pin output for this)
1. Connect 5v power to Geiger board and Arduino (these boards have 3pin output for this)
1. Connect Geiger counter pin to Arudino, INT0, either pin 2 or 3 (see https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/)
1. Connect USB and monitor Serial port, 9600 BAUD (you can use ArduinoIDE inbuilt Serial Monitor, or something like PuTTY)
