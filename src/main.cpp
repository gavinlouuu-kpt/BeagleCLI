#include <Arduino.h>
#include <Wire.h>
#include <LittleFS.h>
#include <FS.h>
#include <pinConfig.h>
#include <Init.h>
#include <FirebaseJson.h>
#include <SD.h>
#include <WiFi.h>
#include <Update.h>
#include <time.h>
#include <map>
#include <vector>
#include <Hardware.h>
#include <beagleCLI.h>
#include <Network.h>
#include <SPI.h>
#include <Firebase_ESP_Client.h>
#include <vector>

#define BLYNK_TEMPLATE_ID "TMPL6NuCPaz5v"
#define BLYNK_TEMPLATE_NAME "ESP32 Gas Sensor Platform"
#define BLYNK_AUTH_TOKEN "9T8WKE1DMtX6qm1oWR4OcZK7_6OMvE_i"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

char ssid[] = "3Broadband_AA0A";
char pass[] = "7WCHEAFF52";

int U2_RX = 16;
int U2_TX = 17;

std::vector<uint8_t> modbus_req = {0x01, 0x03, 0x00, 0x05, 0x00, 0x01, 0x94, 0x0B};
std::vector<uint8_t> modbus_on = {0xFF, 0x01, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0xFB};
std::vector<uint8_t> actv_msg_on = {0xFF, 0x01, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0xFC};

int sensorVal;

BlynkTimer timer;

void myTimer()
{
  // This function describes what will happen with each timer tick
  // e.g. writing sensor value to datastream V5
  Blynk.virtualWrite(V0, sensorVal);
  Blynk.virtualWrite(V1, sensorVal);
}

void setup()
{

  Serial.begin(115200);                          /* prepare for possible serial debug */
  Serial2.begin(9600, SERIAL_8N1, U2_RX, U2_TX); /* prepare for possible serial debug */
  // Serial2.write(modbus_on.data(), modbus_on.size());
  Serial2.write(actv_msg_on.data(), actv_msg_on.size());
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(2000L, myTimer); // set timer for 1 second
  //   // Initialize LittleFS
  // if (!LittleFS.begin()) {
  //   Serial.println("LittleFS mount failed, formatting...");
  //   // If mounting fails, format LittleFS
  //   if (LittleFS.format()) {
  //     Serial.println("LittleFS formatted successfully");
  //     // Try to mount again after formatting
  //     if (LittleFS.begin()) {
  //       Serial.println("LittleFS mounted successfully after format");
  //     } else {
  //       Serial.println("LittleFS mount failed after format");
  //     }
  //   } else {
  //     Serial.println("LittleFS format failed");
  //   }
  // } else {
  //   Serial.println("LittleFS mounted successfully");
  // }
  // Wire.begin(C_SCL, C_SDA); // DAT2 is SDA, DAT3 is SCL
  // pinSetup();
  // pwmSetup();
  // configInit();
  // cmdSetup();
}

int getPPMFromData(String data)
{
  int ppmPos = data.indexOf("ppm");
  String substr = data.substring(2, ppmPos-1);
  sensorVal = substr.toInt(); // Convert the collected numeric string to an integer
  return sensorVal;
}

void actv_msg()
{
  static String message = ""; // Store the incoming message
  if (Serial2.available())
  {
    char sensorData = Serial2.read(); // Read the incoming byte as a char
    if (sensorData == '\n')
    {
      // got the whole line, parse now
      int ppm = getPPMFromData(message);
      Serial.println(sensorVal);
      message = ""; // Clear the message string
    }
    else
    {
      // cache the message byte
      message += sensorData;
    }

    delay(100);
  }
}

void loop()
{
  Blynk.run();
  timer.run();
  actv_msg();
  // Serial.println(sensorVal);
  // beagleCLI();
  // put your main code here, to run repeatedly:
}
