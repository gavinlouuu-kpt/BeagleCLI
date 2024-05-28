#include <Arduino.h>
#include <Wire.h>
// #include <FS.h>
// #include <pinConfig.h>
// #include <Init.h>
#include <FirebaseJson.h>
#include <WiFi.h>
#include <Update.h>
#include <time.h>
#include <map>
#include <vector>
// #include <Hardware.h>
#include <beagleCLI.h>
// #include <Network.h>
// #include <SPI.h>
#include <Firebase_ESP_Client.h>
#include <M5Stack.h>
#include <exp_setup.h>

// #include <SensorData.h>
// #include <SensorDataFactory.h>

int U2_RX = 16;
int U2_TX = 17;

// void sensorCheck()
// {
//   unsigned long startTime, endTime;
//   startTime = millis();
//   SensorDataFactory factory;
//   SensorData sensorData = factory.createSensorData();
//   endTime = millis();
//   std::vector<uint32_t> data150 = sensorData.getDataVec150();
//   std::vector<uint32_t> data200 = sensorData.getDataVec200();
//   std::vector<uint32_t> data250 = sensorData.getDataVec250();
//   std::vector<uint32_t> data300 = sensorData.getDataVec300();
//   std::vector<uint32_t> data350 = sensorData.getDataVec350();
//   std::vector<uint32_t> data400 = sensorData.getDataVec400();
//   std::vector<uint32_t> data450 = sensorData.getDataVec450();
//   std::vector<uint32_t> data500 = sensorData.getDataVec500();

//   Serial.println("Data150_size_" + String(data150.size()) + " elements");
//   // create a loop to print the data200 vector
//   for (int i = 0; i < data150.size(); i++)
//   {
//     Serial.println(data150[i]);
//   }

//   Serial.println("Data200_size_" + String(data200.size()) + " elements");
//   // create a loop to print the data200 vector
//   for (int i = 0; i < data200.size(); i++)
//   {
//     Serial.println(data200[i]);
//   }

//   Serial.println("Data250_size_" + String(data250.size()));
//   // create a loop to print the data200 vector
//   for (int i = 0; i < data250.size(); i++)
//   {
//     Serial.println(data250[i]);
//   }

//   Serial.println("Data300_size_" + String(data300.size()));
//   // create a loop to print the data300 vector
//   for (int i = 0; i < data300.size(); i++)
//   {
//     Serial.println(data300[i]);
//   }

//   Serial.println("Data350_size_" + String(data350.size()));
//   // create a loop to print the data300 vector
//   for (int i = 0; i < data350.size(); i++)
//   {
//     Serial.println(data350[i]);
//   }

//   Serial.println("Data400_size_" + String(data400.size()));
//   // create a loop to print the data400 vector
//   for (int i = 0; i < data400.size(); i++)
//   {
//     Serial.println(data400[i]);
//   }

//   Serial.println("Data450_size_" + String(data450.size()));
//   // create a loop to print the data400 vector
//   for (int i = 0; i < data450.size(); i++)
//   {
//     Serial.println(data450[i]);
//   }

//   Serial.println("Data500_size_" + String(data500.size()));
//   // create a loop to print the data400 vector
//   for (int i = 0; i < data500.size(); i++)
//   {
//     Serial.println(data500[i]);
//   }

//   Serial.println("Time taken to create SensorData object: " + String(endTime - startTime) + " ms");
// }

void M5_example()
{
  // M5.begin();
  if (!SD.begin())
  { // Initialize the SD card. 初始化SD卡
    M5.Lcd.println(
        "Card failed, or not present"); // Print a message if the SD card
                                        // initialization fails or if the
                                        // SD card does not exist
                                        // 如果SD卡初始化失败或者SD卡不存在，则打印消息
    while (1)
      ;
  }
  M5.Lcd.println("TF card initialized.");
  if (SD.exists("/hello.txt"))
  { // Check if the "/hello.txt" file
    // exists.查看是否存在"/hello.txt"文件
    M5.Lcd.println("hello.txt exists.");
  }
  else
  {
    M5.Lcd.println("hello.txt doesn't exist.");
  }
  M5.Lcd.println("Creating hello.txt");
  File myFile = SD.open("/hello.txt",
                        FILE_WRITE); // Create a new file "/hello.txt".
                                     // 创建一个新文件"/hello.txt"
  if (myFile)
  { // If the file is open, then write to it.
    // 如果文件打开,则进行写入操作
    M5.Lcd.println("Writing to test.txt...");
    myFile.println("SD test.");
    myFile.close(); // Close the file. 关闭文件
    M5.Lcd.println("done.");
  }
  else
  {
    M5.Lcd.println("error opening test.txt");
  }
  delay(500);
  myFile = SD.open("/hello.txt",
                   FILE_READ); // Open the file "/hello.txt" in read mode.
                               // 以读取模式打开文件"/hello.txt"
  if (myFile)
  {
    M5.Lcd.println("/hello.txt Content:");
    // Read the data from the file and print it until the reading is
    // complete. 从文件里读取数据并打印到串口,直到读取完成.
    while (myFile.available())
    {
      M5.Lcd.write(myFile.read());
    }
    myFile.close();
  }
  else
  {
    M5.Lcd.println("error opening /hello.txt"); // If the file is not open.
                                                // 如果文件没有打开
  }
}

void setup()
{

  Serial.begin(115200);
  Serial2.begin(38400, SERIAL_8N1, U2_RX, U2_TX);
  M5.begin(1, 1, 1, 1);
  /* the setups are not not needed now
  pinSetup(); //something in pin setup is causing sd card to not initialize properly
  pwmSetup(); //something in pwm setup is causing sd card to not initialize properly
  */
  cmdSetup();
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
