#include <Arduino.h>
#include <Wire.h>
// #include <FS.h>
#include <pinConfig.h>
#include <Init.h>
#include <FirebaseJson.h>
#include <WiFi.h>
#include <Update.h>
#include <time.h>
#include <map>
#include <vector>
#include <Hardware.h>
#include <beagleCLI.h>
#include <Network.h>
// #include <SPI.h>
#include <Firebase_ESP_Client.h>
#include <M5CoreS3.h>
// #include <M5Unified.h>
#include <exp_setup.h>

// #include <SensorData.h>
// #include <SensorDataFactory.h>
// #include <SensorDataFactory.cpp>

void setup()
{

  Serial.begin(115200);
  Serial2.begin(38400, SERIAL_8N1, U2_RX, U2_TX);
  M5.begin();
  //  the setups are not not needed now
  pinSetup(); // something in pin setup is causing sd card to not initialize properly
  pwmSetup(); // something in pwm setup is causing sd card to not initialize properly

  expMutexSetup();

  cmdSetup();
  networkCheck();

  // sampleTask();
  // M5_example();
  // const char *filename = "/expSetup.json";
  // String configData;
  // M5_SD_JSON(filename, configData);

  // sensorCheck();
}

void loop()
{
  beagleCLI();
}
