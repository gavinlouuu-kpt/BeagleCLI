#include <Arduino.h>
#include <Network.h>
#include <WiFi.h>
#include <time.h>
#include <Init.h>
#include <LittleFS.h>
#include <FirebaseJson.h>
#include <map>
#include <beagleCLI.h>

// std::map<String, std::function<void()>> commandMap;

WiFiManager::WiFiManager() {}

void WiFiManager::ManageWIFI() {
    this->scanNetworks();

    if (WiFi.status() != WL_CONNECTED) {
        String ssid = this->selectNetwork();
        if (ssid != "") {
            String password = this->inputPassword();
            if (this->connectToWiFi(ssid, password)) {
                File file = LittleFS.open("/wifi_credentials.txt", "w");
                if (file) {
                    file.println(ssid);
                    file.println(password);
                    file.close();
                    Serial.println("WiFi credentials saved to LittleFS.");
                } else {
                    Serial.println("Failed to open file for writing.");
                }
            } else {
                Serial.println("Failed to connect to WiFi.");
            }
        }
    } else {
        Serial.println("Already connected to a WiFi network.");
    }
}

void WiFiManager::scanNetworks() {
    Serial.println("Scanning WiFi networks...");
    int numNetworks = WiFi.scanNetworks();
    if (numNetworks == 0) {
        Serial.println("No networks found. Try again.");
        return; // Early return if no networks are found
    } else {
        Serial.print(numNetworks);
        Serial.println(" networks found:");
        for (int i = 0; i < numNetworks; i++) {
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

    // After listing all networks, then check for stored credentials
    if (LittleFS.exists("/wifi_credentials.txt")) {
        File file = LittleFS.open("/wifi_credentials.txt", "r");
        if (file) {
            String storedSSID = file.readStringUntil('\n');
            storedSSID.trim(); // Remove any trailing whitespace or newline characters
            String storedPassword = file.readStringUntil('\n');              
            storedPassword.trim(); // Same here
            file.close();

            // Try to automatically connect if a known network is found
            for (int i = 0; i < numNetworks; i++) {
                if (WiFi.SSID(i) == storedSSID) {
                    Serial.println("Found stored network, attempting to connect...");
                    if (connectToWiFi(storedSSID, storedPassword)) {
                        Serial.println("Successfully connected to stored network.");
                        return; // Exit if successfully connected
                    } else {
                        Serial.println("Failed to connect to stored network.");
                        break; // Break from the loop if connection failed
                    }
                }
            }
        } else {
            Serial.println("Credentials file could not be opened.");
        }
    } else {
        Serial.println("Credentials file does not exist. Proceeding to manual connection.");
    }
    // Add logic here to prompt user for manual connection if automatic connection was not successful
}


String WiFiManager::selectNetwork() {
    int attempt = 0;
    int maxAttempts = 3; // Maximum number of attempts to find the network
    while (attempt < maxAttempts) {
      Serial.println("Enter the number of the network you want to connect to (or 0 to rescan):");
      while (!Serial.available());
      int networkNum = Serial.parseInt();
      
      if (networkNum == 0) {
        scanNetworks(); // Rescan networks
        attempt++;
        continue;
      }

      if (networkNum < 0 || networkNum > WiFi.scanNetworks()) {
        Serial.println("Invalid network number. Please try again.");
      } else {
        return WiFi.SSID(networkNum - 1); // Return the selected SSID
      }
    }
    Serial.println("Maximum attempts reached. Please restart the process.");
    return ""; // Return empty string if max attempts reached without selection
  }

String WiFiManager::inputPassword() {
    Serial.println("Enter password:");
    while (!Serial.available());
    String password = Serial.readStringUntil('\n');
    password.trim(); // Remove any whitespace or newline characters
    return password;
  }


bool WiFiManager::connectToWiFi(const String& ssid, const String& password) {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long startTime = millis(); // Get the current time
    const unsigned long timeout = 30000; // Set timeout duration (e.g., 30000 milliseconds or 30 seconds)

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");

        if (millis() - startTime >= timeout) { // Check if the timeout has been reached
            Serial.println("\nConnection Timeout!");
            return false; // Return false if the timeout is reached
        }
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        configTime(gmtOffset_sec, daylightOffset_sec, "hk.pool.ntp.org","asia.pool.ntp.org","time.nist.gov"); // Initialize NTP
        return true; // Return true if connected
    } else {
        Serial.println("\nFailed to connect. Please try again.");
        return false; // Return false if not connected
    }
}

void networkState(){
    char* hostname = "www.google.com";
    IPAddress resolvedIP;
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi connection lost.");
    } 
    else {
    Serial.println("WiFi connected.");
        if(WiFi.hostByName(hostname, resolvedIP)) {
            Serial.print("IP Address for ");
            Serial.print(hostname);
            Serial.print(" is: ");
            Serial.println(resolvedIP);
        } 
        else {
            Serial.print("DNS Failed for ");
            Serial.println(hostname);
        }
    }
}


void networkCMD(){
    commandMap["net"] = []() { networkState(); };
    commandMap["wifi"] = []() { WiFiManager wifiManager; wifiManager.ManageWIFI(); };

    
}