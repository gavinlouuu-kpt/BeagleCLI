#include <Arduino.h>
#include <FirebaseJson.h>
#include <LittleFS.h>
#include <FS.h>
// #include <lvgl.h>
#include <string>
#include <pinConfig.h>

int pumpSpeed;
String FIREBASE_PROJECT_ID;
String USER_EMAIL;
String USER_PASSWORD;
String DATABASE_URL;
String API_KEY;
String STORAGE_BUCKET_ID;
long gmtOffset_sec;
int daylightOffset_sec;
int FIREBASE_PATH;
String TARGET_GROUP;
String USER_ID;


// // LittleFS system
void writeFile(const char *path, const char *message) {
  File file = LittleFS.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}


// Function to read a generic value from a JSON file and return it as a String
String readConfigValue(const char *path, const char *jsonPath) {
  File file = LittleFS.open(path, FILE_READ);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return ""; // Return an empty string to indicate failure
  }

  // Read the content of the file into a string
  String fileContent;
  while (file.available()) {
    fileContent += char(file.read());
  }
  file.close();

  // Use FirebaseJson to parse the JSON string
  FirebaseJson json;
  json.setJsonData(fileContent);

  // Create a FirebaseJsonData object to store the result
  FirebaseJsonData jsonData;

  // Extract the value
  if (!json.get(jsonData, jsonPath)) {
    Serial.print("Failed to read ");
    Serial.print(jsonPath);
    Serial.println(" from JSON");
    return ""; // Return an empty string to indicate failure
  }

  // Return the value as a String
  return jsonData.stringValue;
}

void configIntMod(const char *path, int value) {
  // Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }
  
  // Open the config.json file for reading
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config.json for reading");
    return;
  }

  // Read the file into a String
  String configContent;
  while (configFile.available()) {
    configContent += char(configFile.read());
  }
  configFile.close(); // Close the file after reading

  // Parse the JSON content
  FirebaseJson json;
  json.setJsonData(configContent.c_str());

  // Check and modify the target
  FirebaseJsonData jsonData;
  if (json.get(jsonData, path)) {
    // Target exists, modify its value
    json.set(path, value);

    // Serialize the modified JSON object
    String modifiedConfig;
    json.toString(modifiedConfig, true);

    // Open the config.json file for writing
    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("Failed to open config.json for writing");
      return;
    }

    // Write the modified configuration back to the file
    configFile.print(modifiedConfig);
    configFile.close(); // Close the file after writing
    Serial.println("Configuration updated successfully.");
  } else {
    Serial.println("Target path does not exist in the configuration.");
  }

  // Cleanup LittleFS but it closes the FS entirely needs to be implemented system wide
  // LittleFS.end();
}

void configInit(){
  ledcWrite(PumpPWM, 0); // turn off pump
  // USER_ID = readConfigValue("/config.json", "/USER_ID");
  // TARGET_GROUP = readConfigValue("/config.json", "/TARGET_GROUP");
  // FIREBASE_PATH = readConfigValue("/config.json", "/FIREBASE_PATH").toInt();
  // pumpSpeed = readConfigValue("/config.json","/pump_speed").toInt();
  // FIREBASE_PROJECT_ID = readConfigValue("/config.json", "/FIREBASE_PROJECT_ID");
  // STORAGE_BUCKET_ID = readConfigValue("/config.json", "/STORAGE_BUCKET_ID");
  // USER_EMAIL = readConfigValue("/config.json", "/USER_EMAIL");
  // USER_PASSWORD = readConfigValue("/config.json", "/USER_PASSWORD");
  // DATABASE_URL = readConfigValue("/config.json", "/DATABASE_URL");	
  // API_KEY = readConfigValue("/config.json", "/API_KEY");
  std::string gmtOffset_sec_str = readConfigValue("/config.json", "/gmtOffset_sec").c_str();
  gmtOffset_sec = std::stol(gmtOffset_sec_str);
  daylightOffset_sec = readConfigValue("/config.json", "/daylightOffset_sec").toInt();
  
  
  //other use cases
  // String isEnabledStr = readConfigValue("/config.json", "/is_enabled");
  // bool isEnabled = isEnabledStr.equalsIgnoreCase("true"); // Simple way to convert string to boolean
  // Serial.print("Is Enabled: ");
  // Serial.println(isEnabled ? "True" : "False");
  // String temperatureStr = readConfigValue("/config.json", "/temperature");
  // float temperature = temperatureStr.toFloat(); // Convert string to float
  // Serial.print("Temperature: ");
  // Serial.println(temperature);
}

// // Factory Reset
void FactoryReset(){
  // read file at "/factory/preset.json" retrieve pumpSpeed and write the speed to "/config.json"
  // a factory reset is essentially copying preset.json to config.json
  // if the file exists, delete it
  if (LittleFS.exists("/config.json")) {
    LittleFS.remove("/config.json");
  }
  // copy the file
  if (LittleFS.exists("/factory/preset.json")) {
    File file = LittleFS.open("/factory/preset.json", FILE_READ);
    if (!file) {
      Serial.println("Failed to open file for reading");
      return;
    }
    String fileContent;
    while (file.available()) {
      fileContent += char(file.read());
    }
    file.close();
    writeFile("/config.json", fileContent.c_str());
  }
}


// Initialize Device
