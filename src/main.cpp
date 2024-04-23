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


#include "CRC16.h"

CRC16 crc(CRC16_MODBUS_POLYNOME,
          CRC16_MODBUS_INITIAL,
          CRC16_MODBUS_XOR_OUT,
          CRC16_MODBUS_REV_IN,
          CRC16_MODBUS_REV_OUT);



void modbus_crc()
{
  uint8_t arr[8] = { 0x01, 0x06, 0x00, 0x34, 0x00, 0x01, 0x00, 0x00 }; // Increased size for CRC

  // Add only the relevant data to CRC calculation
  for (int i = 0; i < 6; i++) { 
    crc.add(arr[i]);
  }

  uint16_t crcValue = crc.calc(); // Store the calculated CRC

  // Append CRC in little-endian order
  arr[6] = crcValue & 0xFF; // Lower byte
  arr[7] = (crcValue >> 8) & 0xFF; // Higher byte

  Serial.println(crcValue, HEX); // Print the CRC value

  Serial2.write(arr, sizeof(arr)); // Send the array including the CRC
}

void modbus_off()
{
  uint8_t arr[8] = { 0x01, 0x06, 0x00, 0x34, 0x00, 0x00, 0x00, 0x00 }; // Increased size for CRC

  // Add only the relevant data to CRC calculation
  for (int i = 0; i < 6; i++) { 
    crc.add(arr[i]);
  }

  uint16_t crcValue = crc.calc(); // Store the calculated CRC

  // Append CRC in little-endian order
  arr[6] = crcValue & 0xFF; // Lower byte
  arr[7] = (crcValue >> 8) & 0xFF; // Higher byte

  Serial.println(crcValue, HEX); // Print the CRC value

  Serial2.write(arr, sizeof(arr)); // Send the array including the CRC
}


int U2_RX = 16;
int U2_TX = 17;


void setup() {

  Serial.begin( 115200 ); /* prepare for possible serial debug */
  Serial2.begin( 38400, SERIAL_8N1, U2_RX, U2_TX ); /* prepare for possible serial debug */
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
  // modbus_crc();
  modbus_off();
}

void loop() {
  beagleCLI();
  // put your main code here, to run repeatedly:
}

