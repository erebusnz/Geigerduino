/*
   Geigerduino AWS IOT Serial Geiger–Müller Counter for ESP32

   @author stig@sdm.nz
   @version 0.1
   @license GNU GPLv3

   Compatible with RadiationD-v1.1-CAJOE, 52DZ-DIY.com marked boards
   Note that these boards are 5v TTL (NOT 3.3v) so you need 5v Arduino (such as micro)

   Conversion factor is for J305 marked tube only. Update to match your tube.
   See https://www.cooking-hacks.com/documentation/tutorials/geiger-counter-radiation-sensor-board-arduino-raspberry-pi-tutorial#cpm_to_servants
*/

#include <SPI.h>
#include <limits.h>
#include <AWS_IOT.h>
#include <WiFi.h>
#include "wifi_aws_credentials.h"

//User variables
const int interrupt_pin = 0;             //GM tube interrupt pin
const float conversion_factor =  0.0057; //GM tube SBM-20, see https://www.cooking-hacks.com/documentation/tutorials/geiger-counter-radiation-sensor-board-arduino-raspberry-pi-tutorial#cpm_to_servants

//Application
#define LOG_FREQUENCY 1000 // 1 second
#define TX_FREQUENCY 30000 // 30 seconds
#define LOGGER_SIZE 60 // 60 data points
#define TARGET_OBS_RATE 1200 // Ensure 60 observations @ 20 CPM (60*20)
#define MIN_OBS 3 // Minimum observation to send CPM
#define MAX_OBS 60 // Maximum observations to send CPM

hw_timer_t * timer = NULL;
const int NULLDATA = ULONG_MAX;
volatile unsigned long counts = 0;
volatile unsigned long cpslogger[LOGGER_SIZE];
volatile unsigned long last_cpm = 0;
volatile int log_index = 0;
int last_log_index = 0;
int last_obs = 60;
unsigned long previous_millis = 0;

//IOT
char CLIENT_ID[] = "GeigerduinoIOT-v0.0.1";
AWS_IOT iot_thing;   // AWS_IOT instance
int status = WL_IDLE_STATUS;
int tick = 0, msgCount = 0, msgReceived = 0;
char payload[512];
char rcvdPayload[512];

/**
   recordData

   Logs clicks each second using hardware timer
   for accuracy
*/
void IRAM_ATTR recordData() {
  if (counts == ULONG_MAX)
    counts = ULONG_MAX - 1;
  cpslogger[log_index++] = counts;
  if (log_index >= LOGGER_SIZE)
    log_index = 0;
  counts = 0;
}

/**
   setup()

   Start Geiger using interrupt pin
   Start Timer1 (hardware timer)
*/
void setup() {
  //Start serial
  Serial.begin(9600);

  //Start Geiger
  pinMode(interrupt_pin, INPUT);
  digitalWrite(interrupt_pin, HIGH);
  interrupts();
  attachInterrupt(digitalPinToInterrupt(interrupt_pin), geigerClick, FALLING);

  //Use ULONG_MAX to represent 'NULL data' on startup
  for (int i = 0; i < LOGGER_SIZE; i++)
  {
    cpslogger[i] = NULLDATA;
  }

  //Begin counter reporting
  timer = timerBegin(0, 80, true); //80 = 80Mhz prescaler in ESP32
  timerAttachInterrupt(timer, &recordData, true);
  timerAlarmWrite(timer, 1000 * LOG_FREQUENCY, true);
  timerAlarmEnable(timer);

  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(WIFI_SSID);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    delay(5000); //Wait before connecting
  }

  Serial.println("Connected to wifi");

  if (iot_thing.connect(HOST_ADDRESS, CLIENT_ID) == 0) // Connect to AWS using Host Address and Cliend ID
  {
    Serial.println("Connected to AWS");
  }
  else
  {
    Serial.println("AWS connection failed, Check the HOST Address");
    while (1);
  }

  delay(2000); //Wait before sending data
}

/**
   getData (int size - CPS observations requested, int period - Duration of observation to use for data)

   Returns 'count per period' based on specified quantity of most recent Count Per Second observations
   for a specified period.

   For example this can return 'CPS' using 5 most recent observations (size=5, period=1)
   or 'CPM' using 60 most recent observations (size=60, period=60)

   Maximum size is array length
*/
int getData(int size, int period, int current_index) {
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
  data = (data * (period / observations));
  return data;
}

/**
   Geiger-Mueller Tube Click

   Only increments count to avoid locking
*/
void geigerClick() {
  counts++;
}

/**
   transmitData

   Sends CPS, CPM and radiation dose over Serial
*/
void transmitData() {
  //Obtain log_index at start (volatile)
  int current_index = log_index;
  unsigned long cps = getData(TX_FREQUENCY / (LOG_FREQUENCY * 1.0f), 1, current_index); //X observations / s

  //Find optimal observations for current CPM
  int obs = (TARGET_OBS_RATE * 1.0f) / last_cpm;
  if (obs < MIN_OBS) {
    obs = MIN_OBS;
  } else if (obs > MAX_OBS) {
    obs = MAX_OBS;
  }

  //Limit observation increase so we don't extend observations into earlier high counts when CPM is trending down
  int limit_obs = last_obs + (log_index - last_log_index < 0 ? LOGGER_SIZE + log_index - last_log_index : log_index - last_log_index);
  if (obs > limit_obs)
    obs = limit_obs;

  unsigned long cpm = getData(obs, 60, current_index); //Counts Per Minute is always 60
  last_obs = obs;
  last_log_index = current_index;
  last_cpm = cpm;
  char microseverts[8];
  dtostrf(cpm * conversion_factor, 4, 2, microseverts);
  char buffer[54];
  sprintf(buffer, "CPS,%ld,CPM,%ld,OBS,%1d,uSv/hr,%s\r\n", cps, cpm, obs, microseverts);
  Serial.print(buffer);

  sprintf(payload, "{\"state\": {\"reported\": {\"CPS\": \"%1d\", \"CPM\": \"%ld\", \"OBS\": \"%1d\", \"uSv/hr\": \"%s\"}}}", cps, cpm, obs, microseverts); // Create the payload for publishing

  if (iot_thing.publish(TOPIC_NAME, payload) == 0) // Publish the message(Temp and humidity)
  {
    Serial.print("Publish Message:");
    Serial.println(payload);
  }
  else
  {
    Serial.println("Publish failed");
  }
}

/**
   Main program
*/
void loop() {
  unsigned long current_millis = millis();
  if (current_millis - previous_millis >= TX_FREQUENCY)
  {
    previous_millis = current_millis;
    transmitData();
  }
}
