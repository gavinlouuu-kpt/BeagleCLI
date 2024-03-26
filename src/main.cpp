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

std::map<String, std::function<void()>> commandMap;




void setup() {

  Serial.begin( 115200 ); /* prepare for possible serial debug */
    // Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed, formatting...");
    // If mounting fails, format LittleFS
    if (LittleFS.format()) {
      Serial.println("LittleFS formatted successfully");
      // Try to mount again after formatting
      if (LittleFS.begin()) {
        Serial.println("LittleFS mounted successfully after format");
      } else {
        Serial.println("LittleFS mount failed after format");
      }
    } else {
      Serial.println("LittleFS format failed");
    }
  } else {
    Serial.println("LittleFS mounted successfully");
  }
  Wire.begin(C_SCL, C_SDA); // DAT2 is SDA, DAT3 is SCL
  pinSetup();
  pwmSetup();
  configInit();
  cmdSetup();

}

void loop() {
  beagleCLI();
  // put your main code here, to run repeatedly:
}
