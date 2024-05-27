#include <Arduino.h>
#include <Wire.h>
// #include <LittleFS.h>
#include <FS.h>
#include <pinConfig.h>
#include <Init.h>
#include <FirebaseJson.h>
// #include <SD.h>
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
#include <M5Stack.h>
#include <exp_setup.h>

int U2_RX = 16;
int U2_TX = 17;

// void M5_SD_test()
// {
//   if (!SD.begin())
//   { // Initialize the SD card. 初始化SD卡
//     M5.Lcd.println(
//         "Card failed, or not present");
//     while (1)
//       ;
//   }
//   M5.Lcd.println("TF card initialized.");
//   if (SD.exists("/hello.txt"))
//   { // Check if the "/hello.txt" file
//     // exists.查看是否存在"/hello.txt"文件
//     M5.Lcd.println("hello.txt exists.");
//   }
//   else
//   {
//     M5.Lcd.println("hello.txt doesn't exist.");
//   }
//   M5.Lcd.println("Creating hello.txt");
//   File myFile = SD.open("/hello.txt",
//                         FILE_WRITE); // Create a new file "/hello.txt".
//                                      // 创建一个新文件"/hello.txt"
//   if (myFile)
//   { // If the file is open, then write to it.
//     // 如果文件打开,则进行写入操作
//     M5.Lcd.println("Writing to test.txt...");
//     myFile.println("SD test.");
//     myFile.close(); // Close the file. 关闭文件
//     M5.Lcd.println("done.");
//   }
//   else
//   {
//     M5.Lcd.println("error opening test.txt");
//   }
//   delay(500);
//   myFile = SD.open("/expSetup.json",
//                    FILE_READ); // Open the file "/hello.txt" in read mode.
//                                // 以读取模式打开文件"/hello.txt"
//   if (myFile)
//   {
//     M5.Lcd.println("/expSetup.json Content:");
//     // Read the data from the file and print it until the reading is
//     // complete. 从文件里读取数据并打印到串口,直到读取完成.
//     while (myFile.available())
//     {
//       M5.Lcd.write(myFile.read());
//       // Serial.println(myFile.read());
//     }
//     myFile.close();
//   }
//   else
//   {
//     M5.Lcd.println("error opening /hello.txt"); // If the file is not open.
//                                                 // 如果文件没有打开
//   }
// }

void setup()
{

  Serial.begin(115200);                           /* prepare for possible serial debug */
  Serial2.begin(38400, SERIAL_8N1, U2_RX, U2_TX); /* prepare for possible serial debug */
  // Initialize LittleFS
  // if (!LittleFS.begin())
  // {
  //   Serial.println("LittleFS mount failed, formatting...");
  //   // If mounting fails, format LittleFS
  //   if (LittleFS.format())
  //   {
  //     Serial.println("LittleFS formatted successfully");
  //     // Try to mount again after formatting
  //     if (LittleFS.begin())
  //     {
  //       Serial.println("LittleFS mounted successfully after format");
  //     }
  //     else
  //     {
  //       Serial.println("LittleFS mount failed after format");
  //     }
  //   }
  //   else
  //   {
  //     Serial.println("LittleFS format failed");
  //   }
  // }
  // else
  // {
  //   Serial.println("LittleFS mounted successfully");
  // }
  // Wire.begin(21, 22); // Wire.begin(C_SCL, C_SDA); // DAT2 is SDA, DAT3 is SCL
  M5.begin();
  // M5_SD_test();
  // pinSetup();
  // pwmSetup();
  // configInit();
  // cmdSetup();

  // String configData;
  // const char *filename = "/expSetup.json";

  // load_sd_json(filename, configData);
  // delay(10000);

  // M5_SD_CMD(filename, configData);
  // Serial.println(configData);
  read_number_of_setups();
}

void loop()
{
  beagleCLI();
  // put your main code here, to run repeatedly:
}
