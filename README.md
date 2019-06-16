# Geigerduino
Geigerduino Serial Geiger–Müller Counter

Compatible with RadiationD-v1.1-CAJOE, 52DZ-DIY.com marked boards

Conversion factor is for J305 marked tube only. Update to match your tube.
See https://www.cooking-hacks.com/documentation/tutorials/geiger-counter-radiation-sensor-board-arduino-raspberry-pi-tutorial#cpm_to_servants

Instructions:

# Confirm you have 5v logic-level Arduino! Otherwise you will blow up a 3.3v interupt pin or the Arduino itself.
# Connect Gnd to Gegier board and Arudino (these boards have 3pin output for this)
# Connect 5v power to Geiger board and Arduino (these boards have 3pin output for this)
# Connect Geiger counter pin to Arudino, INT0, either pin 2 or 3 (see https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/)
# Connect USB and monitor Serial port, 9600 BAUD (you can use ArduinoIDE inbuilt Serial Monitor, or something like PuTTY)
