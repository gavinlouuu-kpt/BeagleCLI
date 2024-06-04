#include <chrono>
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

int heatingTime = 8;
std::unordered_map<int, std::vector<uint32_t>> UOM_sensorData;
std::vector<int> heaterSettings = {150, 200, 250, 300, 350, 400, 450, 500};

void displayMap(std::unordered_map<int, std::vector<uint32_t>> UOM_sensorData)
{
    for (auto iter = UOM_sensorData.begin(); iter != UOM_sensorData.end(); ++iter)
    {
        int setting = iter->first;                           // Extract the setting
        const std::vector<uint32_t> &dataVec = iter->second; // Extract the vector of data

        Serial.print("Data for setting ");
        Serial.print(setting);
        Serial.println(": ");

        for (auto value : dataVec)
        {
            Serial.println(value);
        }
        Serial.println(); // Adds a new line after the list of data for readability
    }
    Serial.println("All data printed");
}
// save uom data as csv file in SD card
void saveUOMData(std::unordered_map<int, std::vector<uint32_t>> UOM_sensorData)
{
    // File name for the CSV file
    const char *filename = "/uomData.csv";

    // Open the file in write mode
    File myFile = SD.open(filename, FILE_WRITE);

    // Check if the file was opened successfully
    if (myFile)
    {
        // Write the header row
        myFile.println("Setting,Data");

        // Write the data
        for (auto iter = UOM_sensorData.begin(); iter != UOM_sensorData.end(); ++iter)
        {
            int setting = iter->first;                           // Extract the setting
            const std::vector<uint32_t> &dataVec = iter->second; // Extract the vector of data

            for (auto value : dataVec)
            {
                myFile.print(setting);
                myFile.print(",");
                myFile.println(value);
            }
        }

        // Close the file
        myFile.close();
    }
    else
    {
        Serial.println("Error opening file for writing");
    }
}

// called as RTOS task
int UOM_sensor(std::unordered_map<int, std::vector<uint32_t>> UOM_sensorData, std::vector<int> heaterSettings, int heatingTime)
{
    if (!bme.begin())
    {
        Serial.println("Could not find a valid BME680 sensor, check wiring!");
        while (1)
            ;
    }

    while (expState == EXP_DAQ)
    {
        Serial.println("In the loop");
        for (int setting : heaterSettings)
        {
            Serial.print("Setting the gas heater to ");
            Serial.println(setting);
            bme.setGasHeater(setting, heatingTime);
            if (bme.performReading())
            {
                UOM_sensorData[setting].push_back(bme.gas_resistance);
                Serial.print("Gas Resistance: ");
                Serial.println(bme.gas_resistance);
                Serial.println(expState);
            }
            else
            {
                Serial.println("Failed to perform reading.");
            }
        }
    }

    // Optional: process or display the collected data
    displayMap(UOM_sensorData);
    saveUOMData(UOM_sensorData);

    return 0;
}

void uomTest()
{
    UOM_sensor(UOM_sensorData, heaterSettings, heatingTime);
}

void sample_start(void *pvParameters)
{
    uomTest();
    vTaskDelete(NULL);
}

void sampleTask()
{
    xTaskCreate(
        sample_start,   /* Task function. */
        "sample_start", /* String with name of task. */
        10000,          /* Stack size in bytes. */
        NULL,           /* Parameter passed as input of the task */
        1,              /* Priority of the task. */
        NULL);          /* Task handle. */
}

void expDAQ()
{
    expState = EXP_DAQ;
    Serial.println("Setting expState to state:" + String(expState));
}

void expIDLE()
{
    expState = EXP_IDLE;
    Serial.println("Setting expState to state:" + String(expState));
}

void checkState()
{
    Serial.println("Current state: " + String(expState));
}

void expTest()
{
    c7_on();
    delay(100);
    cmd_on();
    delay(100);
    expDAQ();
    delay(100);
    sampleTask();
    delay(30000);
    c0_on_off();
    delay(60000);
    expIDLE();
    delay(100);
    relay_off();
}

void expTest_2()
{
    c7_on();
    delay(100);
    cmd_on();
    delay(100);
    expDAQ();
    delay(100);
    sampleTask();
    delay(30000);
    c1_on_off();
    delay(60000);
    expIDLE();
    delay(100);
    relay_off();
}

void expTest_3()
{
    c7_on();
    delay(100);
    cmd_on();
    delay(100);
    expDAQ();
    delay(100);
    sampleTask();
    delay(30000);
    c2_on_off();
    delay(60000);
    expIDLE();
    delay(100);
    relay_off();
}

void readConfigCMD()
{
    commandMap["startRTOS"] = []()
    { expTask(); };
    commandMap["sdTest"] = []()
    { exp_build(); };
    commandMap["uomTest"] = []()
    { uomTest(); };
    commandMap["sampleTask"] = []()
    { sampleTask(); };
    commandMap["expDAQ"] = []()
    { expDAQ(); };
    commandMap["expIDLE"] = []()
    { expIDLE(); };
    commandMap["checkState"] = []()
    { checkState(); };
    commandMap["exp_run"] = []()
    { expTest(); };
    commandMap["exp_run2"] = []()
    { expTest_2(); };
    commandMap["exp_run3"] = []()
    { expTest_3(); };
}