#include <exp_setup.h>
#include <zsrelay.h>
#include <FirebaseJson.h>
#include <LittleFS.h>
#include <beagleCLI.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>

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

String load_json(String filename)
{
    // Initialize LittleFS
    if (!LittleFS.begin())
    {
        Serial.println("An error has occurred while mounting LittleFS");
    }

    // Open the JSON configuration file
    File configFile = LittleFS.open(filename, "r");
    if (!configFile)
    {
        Serial.println("Failed to open config file for reading");
    }

    // Read the entire file into a string
    String configData;
    while (configFile.available())
    {
        configData += char(configFile.read());
    }
    configFile.close();

    // Parse the JSON data
    // FirebaseJson json;
    // json.setJsonData(configData);

    return configData;
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

void setup_exe(FirebaseJson config, int setup_count, int exp_time = 10000)
{
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

        // for each channel in the array, run the experiment
        Serial.println("Running experiment for setup: " + String(i));
        for (int j = 0; j < repeat; j++)
        {
            Serial.println("Running experiment for setup: " + String(i) + " repeat: " + String(j));
            for (int channel : channels)
            {
                Serial.println("Running experiment for channel: " + String(channel));
                std::vector<uint8_t> on_command = switchCommand(1, channel, 1);

                Serial.println("On command: ");
                for (uint8_t i : on_command)
                {
                    Serial.print(i);
                }
                Serial.println("");

                Serial.println("Turning on channel: " + String(channel));
                Serial2.flush();
                delay(10);
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
                Serial2.flush();
                delay(10);
                Serial2.write(off_command.data(), off_command.size());
                delay(exp_time);

                // run the experiment
                // run_experiment(channel, duration);
            }
        }
    }
}

void read_number_of_setups()
{
    // File name for the JSON configuration
    String filename = "/expSetup.json";

    String configData = load_json(filename);
    FirebaseJson json;
    json.setJsonData(configData);
    int setupCount = count_setup(json);

    setup_exe(json, setupCount);
}

void readConfigCMD()
{
    commandMap["readConfig"] = []()
    { read_number_of_setups(); };
}