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
#include <pinConfig.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <Adafruit_ADS1X15.h>

#include <unordered_map>
#include <WiFi.h>

int heatingTime;
std::vector<int> heaterSettings;
std::unordered_map<int, std::vector<std::pair<unsigned long, uint32_t>>> UOM_sensorData;
std::unordered_map<int, std::vector<std::pair<unsigned long, std::array<uint32_t, 4>>>> ADS_sensorData;

uint8_t ADSi2c = 0x48;
int last_setup_tracker = -1;
int setup_tracker = 0;
int repeat_tracker = 0;
int channel_tracker = 0;
unsigned long startTime = 0;
String exp_name = "";
String currentPath = "";

TaskHandle_t bmeTaskHandle, adsTaskHandle;
Adafruit_BME680 bme; // I2C
Adafruit_ADS1115 ads;

SemaphoreHandle_t expStateMutex; // Declare a mutex
enum ExpState
{
    EXP_IDLE,
    EXP_WARMING_UP,
    EXP_SAVE,
    EXP_READY,
    EXP_DAQ,

};

ExpState expState = EXP_IDLE;

void expMutexSetup()
{
    // Create the mutex in your setup function
    expStateMutex = xSemaphoreCreateMutex();
}

// Use the mutex when accessing expState
void mutexEdit(ExpState state)
{
    // Take the mutex, waiting indefinitely for it to become available
    xSemaphoreTake(expStateMutex, portMAX_DELAY);
    expState = state;
    // Serial.println("Setting expState to state:" + String(expState));
    // Give the mutex back
    xSemaphoreGive(expStateMutex);
}

// Do the same when reading expState
ExpState getExpState()
{
    // Take the mutex, waiting indefinitely for it to become available
    xSemaphoreTake(expStateMutex, portMAX_DELAY);
    ExpState currentState = expState;
    // Give the mutex back
    xSemaphoreGive(expStateMutex);
    return currentState;
}

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

String getExpName(FirebaseJson json, int setup_no, String target)
{
    // get item within a setup
    FirebaseJsonData jsonData;
    String key = "setup_" + String(setup_no);
    String full_key = key + target;
    String result;
    if (json.get(jsonData, full_key))
    {
        json.get(jsonData, full_key);
        result = jsonData.to<String>().c_str();
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

int count_setup(String jsonString)
{
    size_t setupCount = 0;
    FirebaseJson json;
    FirebaseJsonData jsonData;
    // Serial.println(jsonString);
    json.setJsonData(jsonString);
    // Serial.println("Counting Setups");

    // Assuming setups are named 'setup_1', 'setup_2', ..., 'setup_n'
    for (int i = 1; i <= 10; i++)
    { // Adjust the upper limit as needed
        String key = "setup_" + String(i);
        if (json.get(jsonData, key))
        {
            setupCount++;
        }
        // Serial.println("Setup count: " + String(setupCount));
    }

    return setupCount;
}

int sensorCheck()
{
    if (!bme.begin())
    {
        Serial.println("BME680 sensor not found!");
    }
    if (!bme.begin() && !ads.begin(ADSi2c))
    {
        Serial.println("No sensor found!");
        return 0;
    }
    if (bme.begin())
    {
        Serial.println("BME680 sensor found!");
        BMEsampleTask();
        return 1;
    }
    else if (ads.begin(ADSi2c))
    {
        Serial.println("ADS1115 sensor found!");
        ADSsampleTask();
        return 2;
    }
}

void exp_loop(FirebaseJson config, int setup_count, int exp_time = 10000)
{
    int sensorType = sensorCheck();
    mutexEdit(EXP_WARMING_UP);
    Serial.println("State = EXP_WARMING_UP" + String(expState));
    std::vector<uint8_t> pump_on = switchCommand(1, 47, 1);
    Serial2.write(pump_on.data(), pump_on.size());
    // Serial.println("Pump command: ");
    // for (uint8_t i : pump_on)
    // {
    //     Serial.print(i);
    // }

    for (int i = 1; i <= setup_count; i++)
    {
        setup_tracker = i;
        exp_name = getExpName(config, i, "/exp_name");
        heaterSettings = getArr(config, i, "/heater_settings");
        Serial.println("Heater settings: ");
        for (int i : heaterSettings)
        {
            Serial.print(i);
            Serial.print(",");
        }
        heatingTime = getInt(config, i, "/heating_time(ms)");
        Serial.println("Heating time: " + String(heatingTime));
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
        mutexEdit(EXP_READY); // Set experiment state to READY
        Serial.println("State = EXP_READY" + String(expState));

        // for each channel in the array, run the experiment
        // Serial.println("Running experiment for setup: " + String(i));
        for (int j = 0; j < repeat; j++)
        {
            repeat_tracker = j;
            Serial.println("Running experiment for setup: " + String(i) + " repeat: " + String(j));
            for (int cur_channel : channels)
            {
                startTime = millis();
                channel_tracker = cur_channel;
                // Only proceed if the state is EXP_READY
                // while (expState != EXP_READY)
                // {
                //     // Serial.print("/");
                //     vTaskDelay(pdMS_TO_TICKS(1000));
                // }
                mutexEdit(EXP_DAQ);
                Serial.println("State = EXP_DAQ" + String(expState));
                // RTOS logic that is driven by events
                // use rtos to collect data and flag to signal start and stop for data saving
                Serial.println("Running experiment for channel: " + String(cur_channel));
                std::vector<uint8_t> on_command = switchCommand(1, cur_channel, 1);

                // Serial.println("On command: ");
                // for (uint8_t i : on_command)
                // {
                //     Serial.print(i);
                // }
                // Serial.println("");

                Serial.println("Turning on channel: " + String(cur_channel));
                Serial2.write(on_command.data(), on_command.size());
                delay(duration);
                // flush serial

                std::vector<uint8_t> off_command = switchCommand(1, cur_channel, 0);
                // Serial.println("Off command: ");
                // for (uint8_t i : off_command)
                // {
                //     Serial.print(i);
                // }
                // Serial.println("");
                Serial.println("Turning off channel: " + String(cur_channel));
                Serial2.write(off_command.data(), off_command.size());
                delay(exp_time);
                mutexEdit(EXP_SAVE);
                Serial.println("State = EXP_SAVE" + String(expState));
                while (expState != EXP_READY)
                {
                    // Serial.print("/");
                    vTaskDelay(pdMS_TO_TICKS(1000));
                }
                // wait for data to be saved and expState to be set to EXP_READY
            }
        }
    }
    if (sensorType == 1)
    {
        vTaskDelete(bmeTaskHandle);
    }
    else if (sensorType == 2)
    {
        vTaskDelete(adsTaskHandle);
    }
    std::vector<uint8_t> pump_off = switchCommand(1, 47, 0);
    Serial2.write(pump_off.data(), pump_off.size());
    // Serial.println("Pump off command: ");
    // for (uint8_t i : pump_off)
    // {
    //     Serial.print(i);
    // }
    Serial.println("END of LOOP");
    startTime = 0;
    last_setup_tracker = -1;
    mutexEdit(EXP_IDLE);
}

void exp_build()
{
    // File name for the JSON configuration
    const char *filename = "/expSetup.json";
    String configData;
    M5_SD_JSON(filename, configData);
    FirebaseJson json;
    json.setJsonData(configData);
    int setupCount = count_setup(configData);
    // Serial.println("Number of setups: " + String(setupCount));
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
        20480,       /* Stack size in bytes. */
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
            // M5.Lcd.write(myFile.read());
            // or
            configData += char(myFile.read());
            // DO NOT TURN ON BOTH AT THE SAME TIME
        }
        // Serial.println("SD Config data: ");
        // Serial.println(configData);
        myFile.close();
    }
    else
    {
        M5.Lcd.println("error opening /hello.txt"); // If the file is not open.
                                                    // 如果文件没有打开
    }
}

void displayMap(std::unordered_map<int, std::vector<std::pair<unsigned long, uint32_t>>> UOM_sensorData)
{
    Serial.println("Setting,Timestamp,Data");
    for (auto &entry : UOM_sensorData)
    {
        int setting = entry.first;
        for (auto &data : entry.second)
        {
            unsigned long timestamp = data.first; // Extract timestamp
            uint32_t gasResistance = data.second; // Extract gas resistance
            Serial.print(setting);
            Serial.print(",");
            Serial.print(timestamp);
            Serial.print(",");
            Serial.println(gasResistance);
        }
    }
    Serial.println("Data in object printed");
}

bool ensureDirectoryExists(String path)
{
    if (!SD.exists(path))
    {
        if (SD.mkdir(path.c_str()))
        {
            Serial.println("Directory created: " + path);
            return true;
        }
        else
        {
            Serial.println("Failed to create directory: " + path);
            return false;
        }
    }
    return true;
}

String incrementFolder(String folderPath)
{
    int underscoreIndex = folderPath.lastIndexOf('_');
    int lastNumber = 0;
    String basePart = folderPath;

    // Check if the substring after the last underscore can be converted to a number
    if (underscoreIndex != -1 && underscoreIndex < folderPath.length() - 1)
    {
        String numPart = folderPath.substring(underscoreIndex + 1);
        if (numPart.toInt() != 0 || numPart == "0")
        { // Checks if conversion was successful or is "0"
            lastNumber = numPart.toInt();
            basePart = folderPath.substring(0, underscoreIndex); // Adjust the base part to exclude the number
        }
    }

    // Increment and create a new folder name with the updated number
    String newPath;
    do
    {
        newPath = basePart + "_" + (++lastNumber);
    } while (SD.exists(newPath));

    if (SD.mkdir(newPath.c_str()))
    {
        Serial.println("New directory created: " + newPath);
    }
    else
    {
        Serial.println("Failed to create directory: " + newPath);
    }
    return newPath;
}

String createOrIncrementFolder(String folderPath)
{
    if (!ensureDirectoryExists(folderPath))
    {
        Serial.println("Base directory does not exist and could not be created.");
        return "";
    }
    return incrementFolder(folderPath);
}

void saveUOMData(std::unordered_map<int, std::vector<std::pair<unsigned long, uint32_t>>> &UOM_sensorData, int setup_tracker, int repeat_tracker, int channel_tracker, String exp_name)
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return;
    }
    char today[11];
    strftime(today, sizeof(today), "%Y_%m_%d", &timeinfo);
    char currentTime[9];
    strftime(currentTime, sizeof(currentTime), "%H_%M_%S", &timeinfo);

    String baseDir = "/" + String(today);
    String expDir = baseDir + "/" + exp_name;

    if (!ensureDirectoryExists(baseDir) || !ensureDirectoryExists(expDir))
    {
        Serial.println("Failed to ensure directories exist.");
        return;
    }

    // Check if setup_tracker has changed and update the directory if necessary
    if (setup_tracker != last_setup_tracker)
    {
        currentPath = createOrIncrementFolder(expDir);
        last_setup_tracker = setup_tracker; // Update the last known value of setup_tracker
    }

    String uniqueFilename = currentPath + "/" + String(currentTime) + "s" + String(setup_tracker) + "c" + String(channel_tracker) + "r" + String(repeat_tracker) + ".csv";
    const char *filename = uniqueFilename.c_str();

    File myFile = SD.open(filename, FILE_WRITE);
    if (myFile)
    {
        myFile.println("Setting,Timestamp,Data");
        for (auto &entry : UOM_sensorData)
        {
            int setting = entry.first;
            for (auto &data : entry.second)
            {
                unsigned long timestamp = data.first; // Extract timestamp
                uint32_t gasResistance = data.second; // Extract gas resistance
                myFile.print(setting);
                myFile.print(",");
                myFile.print(timestamp);
                myFile.print(",");
                myFile.println(gasResistance);
            }
        }
        Serial.println("Data saved to file: " + String(filename));
        myFile.close();
    }
    else
    {
        Serial.println("Error opening file for writing");
    }
    UOM_sensorData.clear();
}

void saveADSData(std::unordered_map<int, std::vector<std::pair<unsigned long, std::array<uint32_t, 4>>>> &ADS_sensorData, int setup_tracker, int repeat_tracker, int channel_tracker, String exp_name)
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return;
    }
    char today[11];
    strftime(today, sizeof(today), "%Y_%m_%d", &timeinfo);
    char currentTime[9];
    strftime(currentTime, sizeof(currentTime), "%H_%M_%S", &timeinfo);

    String baseDir = "/" + String(today);
    String expDir = baseDir + "/" + exp_name;

    if (!ensureDirectoryExists(baseDir) || !ensureDirectoryExists(expDir))
    {
        Serial.println("Failed to ensure directories exist.");
        return;
    }

    // Check if setup_tracker has changed and update the directory if necessary
    if (setup_tracker != last_setup_tracker)
    {
        currentPath = createOrIncrementFolder(expDir);
        last_setup_tracker = setup_tracker; // Update the last known value of setup_tracker
    }

    String uniqueFilename = currentPath + "/" + String(currentTime) + "s" + String(setup_tracker) + "c" + String(channel_tracker) + "r" + String(repeat_tracker) + ".csv";
    const char *filename = uniqueFilename.c_str();

    File myFile = SD.open(filename, FILE_WRITE);
    if (myFile)
    {
        myFile.println("Setting,Timestamp,Channel_0,Channel_1,Channel_2,Channel_3");
        for (auto &entry : ADS_sensorData)
        {
            int setting = entry.first;
            for (auto &data : entry.second)
            {
                unsigned long timestamp = data.first; // Extract timestamp
                auto &dataArray = data.second;        // Extract gas resistance

                myFile.print(setting);
                myFile.print(",");
                myFile.print(timestamp);
                myFile.print(",");
                for (uint32_t value : dataArray)
                {
                    myFile.print(value);
                    myFile.print(",");
                }
                myFile.println();
            }
        }
        Serial.println("Data saved to file: " + String(filename));
        myFile.close();
    }
    else
    {
        Serial.println("Error opening file for writing");
    }
    UOM_sensorData.clear();
}

int UOM_sensorBME(std::unordered_map<int, std::vector<std::pair<unsigned long, uint32_t>>> &UOM_sensorData, std::vector<int> heaterSettings, int heatingTime)
{
    if (!bme.begin())
    {
        Serial.println("Could not find a valid BME680 sensor, check wiring!");
        while (1)
            ;
    }
    for (int setting : heaterSettings)
    {
        Serial.print(".");
        bme.setGasHeater(setting, heatingTime);
        if (bme.performReading())
        {
            unsigned long timestamp = millis() - startTime;
            UOM_sensorData[setting].push_back(std::make_pair(timestamp, bme.gas_resistance));
        }
        else
        {
            Serial.println("Failed to perform reading.");
        }
    }

    return 0;
}

void sampleBME(void *pvParameters)
{
    for (;;)
    {
        // uomTest();
        int currentState = getExpState();
        // Serial.print("-");
        if (currentState == EXP_DAQ) // EXP_DAQ)
        {

            UOM_sensorBME(UOM_sensorData, heaterSettings, heatingTime);
        }
        else if (currentState == EXP_SAVE)
        {
            // Serial.print("+");
            // displayMap(UOM_sensorData);
            saveUOMData(UOM_sensorData, setup_tracker, repeat_tracker, channel_tracker, exp_name);
            mutexEdit(EXP_READY);
        }
        // vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void BMEsampleTask()
{
    xTaskCreate(
        sampleBME,       /* Task function. */
        "bme_start",     /* String with name of task. */
        10240,           /* Stack size in bytes. */
        NULL,            /* Parameter passed as input of the task */
        1,               /* Priority of the task. */
        &bmeTaskHandle); /* Task handle. */
}

void ads_heaterSettings(std::vector<int> settings)
{
    heaterSettings = settings;
}

int UOM_sensorADS(std::unordered_map<int, std::vector<std::pair<unsigned long, std::array<uint32_t, 4>>>> &ADS_sensorData, std::vector<int> heaterSettings, int heatingTime)
{
    if (!ads.begin(ADSi2c))
    {
        Serial.println("Could not find a valid ADS1115, check wiring!");
        while (1)
            ;
    }
    for (int setting : heaterSettings)
    {
        Serial.print("+");
        ledcWrite(PWM_Heater, setting);
        // if (bme.performReading())
        // {
        delay(heatingTime);
        unsigned long timestamp = millis() - startTime;
        std::array<uint32_t, 4> ADSreadings = {ads.readADC_SingleEnded(0), ads.readADC_SingleEnded(1), ads.readADC_SingleEnded(2), ads.readADC_SingleEnded(3)};
        ADS_sensorData[setting].push_back(std::make_pair(timestamp, ADSreadings));
        // }
        // else
        // {
        // Serial.println("Failed to perform reading.");
        // }
    }

    return 0;
}

void sampleADS(void *pvParameters)
{
    for (;;)
    {
        // uomTest();
        int currentState = getExpState();
        // Serial.print("-");
        if (currentState == EXP_DAQ) // EXP_DAQ)
        {

            UOM_sensorADS(ADS_sensorData, heaterSettings, heatingTime);
        }
        else if (currentState == EXP_SAVE)
        {
            // Serial.print("+");
            // displayMap(UOM_sensorData);
            saveADSData(ADS_sensorData, setup_tracker, repeat_tracker, channel_tracker, exp_name);
            mutexEdit(EXP_READY);
        }
        // vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void ADSsampleTask()
{
    xTaskCreate(
        sampleADS,       /* Task function. */
        "ads_start",     /* String with name of task. */
        10240,           /* Stack size in bytes. */
        NULL,            /* Parameter passed as input of the task */
        1,               /* Priority of the task. */
        &adsTaskHandle); /* Task handle. */
}

void ADStest()
{
    if (!ads.begin(ADSi2c))
    {
        Serial.println("Could not find a valid ADS1115, check wiring!");
        while (1)
            ;
    }
    for (int i = 0; i < 10; i++)
    {
        Serial.println(ads.readADC_SingleEnded(0));
        delay(1000);
    }
}

void expDAQ()
{
    mutexEdit(EXP_DAQ);
}

void expIDLE()
{
    mutexEdit(EXP_IDLE);
}

void expSAVE()
{
    mutexEdit(EXP_SAVE);
}

void checkState()
{
    Serial.println("Current state: " + String(expState));
}

void readConfigCMD()
{
    commandMap["start"] = []()
    { expTask(); };
    commandMap["sample"] = []()
    { BMEsampleTask(); };
    commandMap["expDAQ"] = []()
    { expDAQ(); };
    commandMap["expIDLE"] = []()
    { expIDLE(); };
    commandMap["expSAVE"] = []()
    { expSAVE(); };
    commandMap["checkState"] = []()
    { checkState(); };
    commandMap["ADStest"] = []()
    { ADStest(); };
}