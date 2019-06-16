/*
 * Geigerduino Serial Geiger–Müller Counter
 * 
 * @author stig@sdm.nz
 * @version 0.1
 * @license GNU GPLv3
 * 
 * Compatible with RadiationD-v1.1-CAJOE, 52DZ-DIY.com marked boards
 * Note that these boards are 5v TTL (NOT 3.3v) so you need 5v Arduino (such as micro)
 * 
 * Conversion factor is for J305 marked tube only. Update to match your tube.
 * See https://www.cooking-hacks.com/documentation/tutorials/geiger-counter-radiation-sensor-board-arduino-raspberry-pi-tutorial#cpm_to_servants
 */

#include <SPI.h>
#include <TimerOne.h>
#include <limits.h>

//User variables
const int interrupt_pin = 3;               //GM tube interrupt pin
const float conversion_factor =  0.008120; //GM tube J305ß, see https://www.cooking-hacks.com/documentation/tutorials/geiger-counter-radiation-sensor-board-arduino-raspberry-pi-tutorial#cpm_to_servants

//Application
#define TX_FREQUENCY 2000 // 2 seconds
#define LOGGER_SIZE 60 // 60 data points
const int NULLDATA = ULONG_MAX;
volatile unsigned long counts = 0;
volatile unsigned long cpslogger[LOGGER_SIZE];
volatile int log_index = 0;
unsigned long previous_millis = 0;

/**
 * setup()
 * 
 * Start Geiger using interrupt pin
 * Start Timer1 (hardware timer)
 */
void setup() {
  //Start serial
  Serial.begin(9600);
  
  //Start Geiger
  pinMode(interrupt_pin, INPUT);
  interrupts();
  attachInterrupt(digitalPinToInterrupt(interrupt_pin), geigerClick, FALLING);

  //Use ULONG_MAX to represent 'NULL data' on startup
  for (int i = 0; i < LOGGER_SIZE; i++)
  {
    cpslogger[i] = NULLDATA;
  }

  //Begin counter reporting
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(recordData);
}

/**
 * getData (int size - CPS observations requested, int period - Duration of observation to use for data)
 * 
 * Returns 'count per period' based on specified quantity of most recent Count Per Second observations
 * for a specified period.
 * 
 * For example this can return 'CPS' using 5 most recent observations (size=5, period=1)
 * or 'CPM' using 60 most recent observations (size=60, period=60)
 * 
 * Maximum size is array length
 */
int getData(int size, int period) {
  //Obtain log_index at start (volatile)
  int current_index = log_index;
  int observations = 0;
  //Max size is logger array length
  if (size > LOGGER_SIZE)
    size = LOGGER_SIZE;
  unsigned long data = 0;
  for (int i = 0; i < size; i++) {
    //Find pointer between 0 to array length to start from and move backwards
    int pointer = (current_index - i < 0) ? (current_index - i) + LOGGER_SIZE : current_index - i;
    if (cpslogger[pointer] != NULLDATA) {
      observations++;
      data = data + cpslogger[pointer];
    }
  }
  // Extrapolate found data to period required
  data = (data * (period/observations));
  return data;
}

/**
 * Geiger-Mueller Tube Click
 * 
 * Only increments count to avoid locking
 */
void geigerClick() {
  counts++;
}

/**
 * recordData
 * 
 * Logs clicks each second using hardware timer
 * for accuracy
 */
void recordData() {
  if (counts == ULONG_MAX)
    counts = ULONG_MAX - 1;
  cpslogger[log_index++] = counts;
  if (log_index >= LOGGER_SIZE)
    log_index = 0;
  counts = 0;
}

/**
 * transmitData 
 * 
 * Sends CPS, CPM and radiation dose over Serial
 */
void transmitData() {
  unsigned long cps = getData(TX_FREQUENCY/1000, 1); //X observations / s
  unsigned long cpm = getData(60, 60); //60 observations / 60s (minute)
  char microseverts[6];
  dtostrf(cpm * conversion_factor, 4, 2, microseverts);
  char buffer[50];
  sprintf(buffer, "CPS,%ld,CPM,%ld,uSv/hr,%s\n", cps, cpm, microseverts);
  Serial.print(buffer);
}

/**
 * Main program
 */
void loop() {
  unsigned long current_millis = millis();
  if (current_millis - previous_millis >= TX_FREQUENCY)
  {
    previous_millis = current_millis;
    transmitData();
  }
}
