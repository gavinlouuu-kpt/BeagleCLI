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
#include <M5Stack.h>
#include <exp_setup.h>

// #include <SensorData.h>
// #include <SensorDataFactory.h>

int U2_RX = 16;
int U2_TX = 17;

// void M5_example()
// {
//   // M5.begin();
//   if (!SD.begin())
//   { // Initialize the SD card. 初始化SD卡
//     M5.Lcd.println(
//         "Card failed, or not present"); // Print a message if the SD card
//                                         // initialization fails or if the
//                                         // SD card does not exist
//                                         // 如果SD卡初始化失败或者SD卡不存在，则打印消息
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
//     M5.Lcd.println("/hello.txt Content:");
//     // Read the data from the file and print it until the reading is
//     // complete. 从文件里读取数据并打印到串口,直到读取完成.
//     while (myFile.available())
//     {
//       M5.Lcd.write(myFile.read());
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

  Serial.begin(115200);
  Serial2.begin(38400, SERIAL_8N1, U2_RX, U2_TX);
  M5.begin(1, 1, 1, 1);
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
