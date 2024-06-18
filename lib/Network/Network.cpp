#include <Arduino.h>
#include <Network.h>
#include <WiFi.h>
#include <time.h>
#include <Init.h>
// #include <LittleFS.h>
#include <FirebaseJson.h>
#include <map>
#include <beagleCLI.h>
#include <SD.h>
#include <ArduinoOTA.h>

TaskHandle_t ntCheckTaskHandler;

void saveWIFICredentialsToSD(const char *ssid, const char *password)
{
    FirebaseJson json;

    // Assuming you want to overwrite existing credentials with new ones
    json.set("/" + String(ssid), String(password));

    // Open file for writing
    File file = SD.open("/wifiCredentials.json", "w");
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }

    // Serialize JSON to file
    String jsonData;
    json.toString(jsonData, true);
    file.print(jsonData);

    // Close the file
    file.close();
}

String loadWIFICredentialsFromSD(String ssid)
{
    if (!SD.begin())
    {
        Serial.println("An Error has occurred while mounting SD");
        return "";
    }

    File file = SD.open("/wifiCredentials.json", "r");
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return "";
    }

    String jsonData = file.readString();
    file.close();

    FirebaseJson json;
    json.setJsonData(jsonData);

    FirebaseJsonData jsonDataObj; // Object to store the retrieved data
    String path = "/" + ssid;     // Construct the path dynamically based on the SSID

    // Use the correct method signature for get()
    if (json.get(jsonDataObj, path.c_str()))
    { // Make sure to use path.c_str() to pass a const char* type
        if (jsonDataObj.typeNum == FirebaseJson::JSON_OBJECT || jsonDataObj.typeNum == FirebaseJson::JSON_STRING)
        {
            // Assuming password is stored as a plain string
            String password = jsonDataObj.stringValue;
            Serial.println("SSID: " + ssid + ", Password: " + password);
            return password;
            // Optionally, connect to WiFi here or handle as needed
        }
        else
        {
            Serial.println("Invalid format for password");
            return "";
        }
    }
    else
    {
        // Serial.println("SSID not found in stored credentials");
        return "";
    }
}

void arduinoOTAsetup()
{

    ArduinoOTA
        .onStart([]()
                 {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_SPIFFS
        type = "filesystem";
      }

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type); })
        .onEnd([]()
               { Serial.println("\nEnd"); })
        .onProgress([](unsigned int progress, unsigned int total)
                    { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
        .onError([](ota_error_t error)
                 {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      } });

    ArduinoOTA.begin();
    Serial.println("OTA Ready");
}

void backgroundWIFI()
{
    // Ensure Wi-Fi is on and set to STA mode
    WiFi.mode(WIFI_STA);

    // Start scanning for networks
    Serial.println("Scanning for Wi-Fi networks...");
    int networkCount = WiFi.scanNetworks();
    if (networkCount == 0)
    {
        Serial.println("No networks found.");
        return;
    }

    // Variables to keep track of the best network
    String bestSSID;
    int bestRSSI = -1000; // Start with a very low RSSI

    // Iterate over found networks
    for (int i = 0; i < networkCount; i++)
    {
        String ssid = WiFi.SSID(i);
        int rssi = WiFi.RSSI(i);

        // Load credentials for the current SSID
        String password = loadWIFICredentialsFromSD(ssid);
        if (password != "" && rssi > bestRSSI)
        {
            // Found a better candidate, update tracking variables
            bestSSID = ssid;
            bestRSSI = rssi;
        }
    }

    // If we found a suitable network, attempt to connect
    if (bestSSID != "")
    {
        Serial.println("Connecting to " + bestSSID);
        WiFi.begin(bestSSID.c_str(), loadWIFICredentialsFromSD(bestSSID).c_str());

        // Wait for connection
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20)
        { // 20 attempts, adjust as needed
            delay(500);
            Serial.print(".");
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("\nConnected to " + bestSSID);
            Serial.println("IP address: " + WiFi.localIP().toString());
            configTime(gmtOffset_sec, daylightOffset_sec, "time.nist.gov", "hk.pool.ntp.org", "asia.pool.ntp.org");
            arduinoOTAsetup();
            //   firebaseSetup();
        }
        else
        {
            Serial.println("\nFailed to connect to any known network");
        }
    }
    else
    {
        Serial.println("No known networks found.");
    }

    // Clean up after scanning
    WiFi.scanDelete();
}

void wifiCheckTask(void *pvParameters)
{
    for (;;)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            ArduinoOTA.handle();
            //   updateLocalTime(); // Update time every minute if background WIFI managed
            //   fbKeepAlive();
        }
        else
        {
            backgroundWIFI();
        }
        vTaskDelay(pdMS_TO_TICKS(10000)); // Check every 10 seconds
    }
}

void networkCheck()
{
    xTaskCreate(
        wifiCheckTask,        /* Task function */
        "WiFiCheckTask",      /* Name of the task */
        10240,                /* Stack size */
        NULL,                 /* Task input parameter */
        1,                    /* Priority of the task */
        &ntCheckTaskHandler); /* Task handle */
}

WiFiManager::WiFiManager() {}

void WiFiManager::ManageWIFI()
{
    this->scanNetworks();

    if (WiFi.status() != WL_CONNECTED)
    {
        String ssid = this->selectNetwork();
        if (ssid != "")
        {
            String password = this->inputPassword();
            if (this->connectToWiFi(ssid, password))
            {
                saveWIFICredentialsToSD(ssid.c_str(), password.c_str());
            }
            else
            {
                Serial.println("Failed to connect to WiFi.");
            }
        }
    }
    else
    {
        Serial.println("Already connected to a WiFi network.");
    }
}

void WiFiManager::scanNetworks()
{
    Serial.println("Scanning WiFi networks...");
    int networkCount = WiFi.scanNetworks();
    if (networkCount == 0)
    {
        Serial.println("No networks found. Try again.");
        return; // Early return if no networks are found
    }
    else
    {
        Serial.print(networkCount);
        Serial.println(" networks found:");
        for (int i = 0; i < networkCount; i++)
        {
            // Print details of each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(" dBm) ");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "[OPEN]" : "[SECURE]");
        }
    }
}

String WiFiManager::selectNetwork()
{
    int attempt = 0;
    int maxAttempts = 3; // Maximum number of attempts to find the network
    while (attempt < maxAttempts)
    {
        Serial.println("Enter the number of the network you want to connect to (or 0 to rescan):");
        while (!Serial.available())
            ;
        int networkNum = Serial.parseInt();

        if (networkNum == 0)
        {
            scanNetworks(); // Rescan networks
            attempt++;
            continue;
        }

        if (networkNum < 0 || networkNum > WiFi.scanNetworks())
        {
            Serial.println("Invalid network number. Please try again.");
        }
        else
        {
            return WiFi.SSID(networkNum - 1); // Return the selected SSID
        }
    }
    Serial.println("Maximum attempts reached. Please restart the process.");
    return ""; // Return empty string if max attempts reached without selection
}

String WiFiManager::inputPassword()
{
    Serial.println("Enter password:");
    while (!Serial.available())
        ;
    String password = Serial.readStringUntil('\n');
    password.trim(); // Remove any whitespace or newline characters
    return password;
}

bool WiFiManager::connectToWiFi(const String &ssid, const String &password)
{
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long startTime = millis();  // Get the current time
    const unsigned long timeout = 30000; // Set timeout duration (e.g., 30000 milliseconds or 30 seconds)

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");

        if (millis() - startTime >= timeout)
        { // Check if the timeout has been reached
            Serial.println("\nConnection Timeout!");
            return false; // Return false if the timeout is reached
        }
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nConnected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        long hkGMToffset = 28800;
        long daylightOffset_sec = 0;
        configTime(hkGMToffset, daylightOffset_sec, "hk.pool.ntp.org", "asia.pool.ntp.org", "time.nist.gov"); // Initialize NTP
        return true;                                                                                          // Return true if connected
    }
    else
    {
        Serial.println("\nFailed to connect. Please try again.");
        return false; // Return false if not connected
    }
}

void networkState()
{
    char *hostname = "www.google.com";
    IPAddress resolvedIP;

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi connection lost.");
    }
    else
    {
        Serial.println("WiFi connected.");
        if (WiFi.hostByName(hostname, resolvedIP))
        {
            Serial.print("IP Address for ");
            Serial.print(hostname);
            Serial.print(" is: ");
            Serial.println(resolvedIP);
        }
        else
        {
            Serial.print("DNS Failed for ");
            Serial.println(hostname);
        }
    }
}

void networkCMD()
{
    commandMap["net"] = []()
    { networkState(); };
    commandMap["wifi"] = []()
    { WiFiManager wifiManager; wifiManager.ManageWIFI(); };
}