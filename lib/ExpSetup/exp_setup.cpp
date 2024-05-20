#include <exp_setup.h>
#include <zsrelay.h>
#include <FirebaseJson.h>
#include <LittleFS.h>
#include <beagleCLI.h>

void read_config(String expSetup)
{
    // Create the filename from the experiment setup parameter
    String filename = expSetup + ".json";

    // Initialize LittleFS
    if (!LittleFS.begin())
    {
        Serial.println("An error has occurred while mounting LittleFS");
        return;
    }

    // Open the JSON configuration file
    File configFile = LittleFS.open(filename, "r");
    if (!configFile)
    {
        Serial.println("Failed to open config file for reading");
        return;
    }

    // Read the entire file into a string
    String configData;
    while (configFile.available())
    {
        configData += char(configFile.read());
    }
    configFile.close();

    // Use FirebaseJson to parse the JSON data
    FirebaseJson json;
    FirebaseJsonData jsonData;

    json.setJsonData(configData);

    // Example of how to extract a specific field from the JSON
    if (json.get(jsonData, "someKey"))
    {
        if (jsonData.typeNum == FirebaseJson::JSON_OBJECT || jsonData.typeNum == FirebaseJson::JSON_ARRAY)
        {
            Serial.println(jsonData.stringValue);
        }
        else
        {
            Serial.print("someKey: ");
            Serial.println(jsonData.stringValue);
        }
    }
    else
    {
        Serial.println("Failed to find key 'someKey' in JSON data");
    }
}

#include <FirebaseJson.h>
#include <LittleFS.h>

#include <FirebaseJson.h>
#include <LittleFS.h>

#include <FirebaseJson.h>
#include <LittleFS.h>

void getItem(FirebaseJson json, int setup_no)
{
    FirebaseJsonData jsonData;
    String key = "setup_" + String(setup_no);
    String dur_key = key + "/duration(s)";
    String rep_key = key + "/repeat";
    String ch_key = key + "/channel";
    if (json.get(jsonData, key))
    {
        json.get(jsonData, dur_key);
        Serial.println("Duration: " + String(jsonData.intValue));
        json.get(jsonData, rep_key);
        Serial.println("Repeat: " + String(jsonData.intValue));
        json.get(jsonData, ch_key);
        Serial.print("Channels: ");
        Serial.println(jsonData.to<String>().c_str());

        // if (jsonData.typeNum == FirebaseJson::JSON_OBJECT || jsonData.typeNum == FirebaseJson::JSON_ARRAY)
        // {
        //     Serial.println(jsonData.stringValue);
        // }
        // else
        // {
        //     Serial.print("someKey: ");
        //     Serial.println(jsonData.stringValue);
        // }
    }
    else
    {
        Serial.println("Failed to find key 'someKey' in JSON data");
    }
}

void read_number_of_setups()
{
    // File name for the JSON configuration
    String filename = "/expSetup.json";

    // Initialize LittleFS
    if (!LittleFS.begin())
    {
        Serial.println("An error has occurred while mounting LittleFS");
        return;
    }

    // Open the JSON configuration file
    File configFile = LittleFS.open(filename, "r");
    if (!configFile)
    {
        Serial.println("Failed to open config file for reading");
        return;
    }

    // Read the entire file into a string
    String configData;
    while (configFile.available())
    {
        configData += char(configFile.read());
    }
    configFile.close();

    // Parse the JSON data
    FirebaseJson json;
    json.setJsonData(configData);

    // Debug print the raw JSON data
    String rawJson;
    json.toString(rawJson, true);
    Serial.println("Raw JSON:");
    Serial.println(rawJson);

    // Count setups
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

    // Output the number of setups found
    Serial.print("Number of setups: ");
    Serial.println(setupCount);
    getItem(json, 1);
}

// after parsing the json generate a sequence of number that can be fed into a function to run the experiment
// [[1,3,5,[1,2,3]],[2,2,5,[1,2,3]],[3,1,5,[1,2,3]]] setup_no, duration, repeat, [channels]

void readConfigCMD()
{
    commandMap["readConfig"] = []()
    { read_number_of_setups(); };
}