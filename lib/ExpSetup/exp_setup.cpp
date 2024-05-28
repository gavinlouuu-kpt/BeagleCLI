#include <exp_setup.h>
#include <zsrelay.h>
#include <FirebaseJson.h>
#include <beagleCLI.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <M5Stack.h>
#include <Arduino.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

#include <unordered_map>
#include <chrono>

Adafruit_BME680 bme; // I2C

enum ExpState
{
    EXP_IDLE,
    EXP_WARMING_UP,
    EXP_READY,
    EXP_DAQ,
    EXP_SAVE
};

ExpState expState = EXP_IDLE;

std::vector<int> stringToArray(const std::string &str)
{
    std::vector<int> result;
    std::stringstream ss(str.substr(1, str.size() - 2)); // Remove the brackets
    std::string item;

    while (getline(ss, item, ','))
    {
        result.push_back(stoi(item));
    }

    return result;
}

int getInt(FirebaseJson json, int setup_no, String target)
{
    // get item within a setup
    FirebaseJsonData jsonData;
    String key = "setup_" + String(setup_no);
    String full_key = key + target;
    int result;
    if (json.get(jsonData, full_key))
    {
        json.get(jsonData, full_key);
        result = String(jsonData.intValue).toInt();
    }
    else
    {
        Serial.println("Failed to find key: " + target);
    }
    return result;
}

std::vector<int> getArr(FirebaseJson json, int setup_no, String target)
{
    FirebaseJsonData jsonData;
    String key = "setup_" + String(setup_no);
    String full_key = key + target;
    std::vector<int> result;
    if (json.get(jsonData, full_key))
    {
        json.get(jsonData, full_key);
        result = stringToArray(jsonData.stringValue.c_str());
    }
    else
    {
        Serial.println("Failed to find key: " + target);
    }
    return result;
}

int count_setup(FirebaseJson json)
{
    size_t setupCount = 0;
    FirebaseJsonData jsonData;

    // Assuming setups are named 'setup_1', 'setup_2', ..., 'setup_n'
    for (int i = 1; i <= 10; i++)
    { // Adjust the upper limit as needed
        String key = "setup_" + String(i);
        if (json.get(jsonData, key))
        {
            setupCount++;
        }
    }

    return setupCount;
}

void exp_loop(FirebaseJson config, int setup_count, int exp_time = 10000)
{
    expState = EXP_WARMING_UP;
    std::vector<uint8_t> pump_on = switchCommand(1, 47, 1);
    Serial2.write(pump_on.data(), pump_on.size());
    Serial.println("Pump command: ");
    for (uint8_t i : pump_on)
    {
        Serial.print(i);
    }

    for (int i = 1; i <= setup_count; i++)
    {
        int duration = getInt(config, i, "/duration(s)");
        duration = duration * 1000;
        int repeat = getInt(config, i, "/repeat");
        std::vector<int> channels = getArr(config, i, "/channel");
        // only continue with the experiment if all three parameters are not 0
        if (duration == 0 || repeat == 0 || channels.size() == 0)
        {
            // if invalid setup found exit the function
            Serial.println("Invalid setup: " + String(i));
            continue;
        }
        expState = EXP_READY;

        // for each channel in the array, run the experiment
        Serial.println("Running experiment for setup: " + String(i));
        for (int j = 0; j < repeat; j++)
        {
            Serial.println("Running experiment for setup: " + String(i) + " repeat: " + String(j));
            for (int channel : channels)
            {
                expState = EXP_DAQ;
                // RTOS logic that is driven by events
                // use rtos to collect data and flag to signal start and stop for data saving
                Serial.println("Running experiment for channel: " + String(channel));
                std::vector<uint8_t> on_command = switchCommand(1, channel, 1);

                Serial.println("On command: ");
                for (uint8_t i : on_command)
                {
                    Serial.print(i);
                }
                Serial.println("");

                Serial.println("Turning on channel: " + String(channel));
                Serial2.write(on_command.data(), on_command.size());
                delay(duration);
                // flush serial

                std::vector<uint8_t> off_command = switchCommand(1, channel, 0);
                Serial.println("Off command: ");
                for (uint8_t i : off_command)
                {
                    Serial.print(i);
                }
                Serial.println("");
                Serial.println("Turning off channel: " + String(channel));
                Serial2.write(off_command.data(), off_command.size());
                delay(exp_time);
                expState = EXP_SAVE;
                // wait for data to be saved and expState to be set to EXP_READY
            }
        }
    }

    std::vector<uint8_t> pump_off = switchCommand(1, 47, 0);
    Serial2.write(pump_off.data(), pump_off.size());
    Serial.println("Pump off command: ");
    for (uint8_t i : pump_off)
    {
        Serial.print(i);
    }
    expState = EXP_IDLE;
}

void exp_build()
{
    // File name for the JSON configuration
    const char *filename = "/expSetup.json";
    String configData;
    M5_SD_JSON(filename, configData);
    FirebaseJson json;
    json.setJsonData(configData);
    int setupCount = count_setup(json);
    exp_loop(json, setupCount);
}

void exp_start(void *pvParameters)
{
    exp_build();
    vTaskDelete(NULL);
}

void expTask()
{
    xTaskCreate(
        exp_start,   /* Task function. */
        "exp_start", /* String with name of task. */
        10000,       /* Stack size in bytes. */
        NULL,        /* Parameter passed as input of the task */
        1,           /* Priority of the task. */
        NULL);       /* Task handle. */
}

void M5_SD_JSON(const char *filename, String &configData)
{
    if (!SD.begin())
    { // Initialize the SD card. 初始化SD卡
        M5.Lcd.println(
            "Card failed, or not present");
        while (1)
            ;
    }

    File myFile = SD.open(filename,
                          FILE_READ); // Open the file "/hello.txt" in read mode.
                                      // 以读取模式打开文件"/hello.txt"
    if (myFile)
    {
        M5.Lcd.println("with cmd /expSetup.json Content:");
        while (myFile.available())
        {
            M5.Lcd.write(myFile.read());
            configData += char(myFile.read());
        }
        // Serial.println(configData);
        myFile.close();
    }
    else
    {
        M5.Lcd.println("error opening /hello.txt"); // If the file is not open.
                                                    // 如果文件没有打开
    }
}

// int UOM_sensor()
// {
//     int heatingTime = 8;
//     std::unordered_map<int, std::vector<uint32_t>> UOM_sensorData;
//     std::vector<int> heaterSettings = {150, 200, 250, 300, 350, 400, 450, 500};

//     // auto end = std::chrono::steady_clock::now() + std::chrono::seconds(30); // Assuming you want to run this for 60 seconds
//     auto start = std::chrono::steady_clock::now();
//     auto end = start + std::chrono::seconds(30);

//     while (std::chrono::steady_clock::now() < end)
//     {
//         for (int setting : heaterSettings)
//         {
//             bme.setGasHeater(setting, heatingTime);
//             if (bme.performReading())
//             {
//                 UOM_sensorData[setting].push_back(bme.gas_resistance);
//                 Serial.println("Data for setting " + String(setting) + ": " + String(bme.gas_resistance));
//             }
//         }
//     }

//     // // Optional: process or display the collected data
//     // for (auto &[setting, dataVec] : UOM_sensorData)
//     // {
//     //     // std::cout << "Data for setting " << setting << ": ";
//     //     Serial.println("Data for setting " + String(setting) + ": ");
//     //     for (auto value : dataVec)
//     //     {
//     //         // std::cout << value << " ";
//     //         Serial.println(value);
//     //     }
//     //     std::cout << std::endl;
//     // }

//     return 0;
// }

void bme_setup()
{
    if (!bme.begin())
    {
        Serial.println("Could not find a valid BME680 sensor, check wiring!");
        while (1)
            ;
    }

    // Set up oversampling and filter initialization
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    // bme.setGasHeater(320, 150); // 320*C for 150 ms
};

void bme_read()
{
    // read bme data while expState = EXP_DAQ
    // continue to push data into the sensorData vector
    // if expState = EXP_SAVE, save the data to the SD card

    if (expState == EXP_DAQ)
    {
        bme.performReading();
        Serial.print(bme.temperature);
        Serial.print(bme.pressure / 100.0);
        Serial.print(bme.humidity);
        Serial.print(bme.gas_resistance / 1000.0);
        Serial.println(" KOhms");

        // save data to the sensorData vector
    }
    else if (expState == EXP_SAVE)
    {
        // save data to the SD card
    }
}

void readConfigCMD()
{
    commandMap["startRTOS"] = []()
    { expTask(); };
    commandMap["sdTest"] = []()
    { exp_build(); };
    // commandMap["UOM_sensor"] = []()
    // { UOM_sensor(); };
}