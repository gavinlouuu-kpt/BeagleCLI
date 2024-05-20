// beagleCLI.cpp
#include <WiFi.h>
#include <FirebaseJson.h>
#include <LittleFS.h>
#include <map>
#include <Arduino.h>
#include <stdio.h>
#include <string>
#include <beagleCLI.h>
#include <Firebase_ESP_Client.h>
#include <Network.h>
#include <Wire.h>
#include <Init.h>
#include <pinConfig.h>
#include <Hardware.h>
#include <functional>
#include <SensorDataFactory.h>
#include <zsrelay.h>
#include <exp_setup.h>

std::map<String, std::function<void()>> commandMap;
using CommandHandler = std::function<void(int)>;
std::map<String, CommandHandler> commandHandlers;

void registerCommand(const String &command, CommandHandler handler)
{
    commandHandlers[command] = handler;
}

String processCommand(const String &receivedCommand)
{
    // First, try to execute direct execution commands
    auto cmdIt = commandMap.find(receivedCommand);
    if (cmdIt != commandMap.end())
    {
        cmdIt->second();
        Serial.println("\nCommand executed");
        return "tc"; // task complete
    }

    // Process commands with parameters
    int spaceIndex = receivedCommand.indexOf(' ');
    String command = receivedCommand.substring(0, spaceIndex);
    String paramString = receivedCommand.substring(spaceIndex + 1);

    auto handlerIt = commandHandlers.find(command);
    if (handlerIt != commandHandlers.end() && spaceIndex != -1)
    {
        int param = paramString.toInt();
        handlerIt->second(param);
        Serial.println("\nCommand with parameter executed");
        return "tc";
    }
    else
    {
        Serial.println("\nUnknown command");
        return "uc"; // unknown command
    }
}

void ESPinfo()
{
    uint32_t flash_size = ESP.getFlashChipSize();
    Serial.print("Flash size: ");
    Serial.print(flash_size);
    Serial.println(" bytes");
}

// String processCommand(const String& receivedCommand) {
//     auto it = commandMap.find(receivedCommand);
//     if (it != commandMap.end()) {
//         it->second(); // execute the command
//         Serial.println();
//         Serial.println("Command executed");
//         return "tc"; // task complete
//     } else {
//         Serial.println();
//         Serial.println("Unknown command");
//         return "uc"; // unknown command
//     }
// }

String readSerialInput()
{
    String input = "";
    while (!Serial.available())
    {
        delay(10); // small delay to allow buffer to fill
    }
    while (Serial.available())
    {
        char c = Serial.read();
        if (c == '\n')
        {
            break;
        }
        input += c;
        delay(10); // small delay to allow buffer to fill
    }
    return input;
}

void printFileContent()
{
    Serial.println("Enter the file name to open:");
    String fileName = readSerialInput();
    Serial.println("Opening file: " + fileName);

    File file = LittleFS.open("/" + fileName, "r");
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.println("Contents of the file:");
    while (file.available())
    {
        Serial.write(file.read());
    }
    file.close();
}

void printHexFileContent()
{
    Serial.println("Enter the file name to open:");
    String fileName = readSerialInput(); // Make sure this function implements a way to read input from Serial.
    Serial.println("Opening file: " + fileName);

    File file = LittleFS.open("/" + fileName, "r");
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.println("Contents of the file (hex):");
    while (file.available())
    {
        char c = file.read();
        // Print each byte as a 2-digit hexadecimal number.
        Serial.print("0x");
        if ((uint8_t)c < 0x10)
            Serial.print("0"); // Add leading zero for numbers less than 0x10
        Serial.print((uint8_t)c, HEX);
        Serial.print(" ");
    }
    Serial.println(); // New line after printing file content
    file.close();
}

void listFilesInDirectory(const String &directoryPath)
{
    File dir = LittleFS.open(directoryPath);
    if (!dir)
    {
        Serial.println("Failed to open directory");
        return;
    }
    if (!dir.isDirectory())
    {
        Serial.println("Not a directory");
        return;
    }

    Serial.println("Listing directory: " + directoryPath);
    File file = dir.openNextFile();
    while (file)
    {
        String filePath = file.name();

        // Ensure the file path starts with '/'
        if (!filePath.startsWith("/"))
        {
            filePath = "/" + filePath;
        }

        if (file.isDirectory())
        {
            Serial.print("DIR : ");
            Serial.println(filePath);
            // Recursively list nested directories
            listFilesInDirectory(filePath);
        }
        else
        {
            Serial.print("FILE: ");
            Serial.println(filePath);
        }
        file = dir.openNextFile();
    }
}

bool deleteAllFilesInLittleFS()
{
    return deleteAllFilesInDirectory("/");
}

bool deleteAllFilesInDirectory(const char *dirPath)
{
    File dir = LittleFS.open(dirPath);
    if (!dir || !dir.isDirectory())
    {
        Serial.println(String("Failed to open directory: ") + dirPath);
        return false;
    }

    File file = dir.openNextFile();
    while (file)
    {
        String filePath;
        if (String(dirPath) == "/")
        {
            filePath = "/" + String(file.name());
        }
        else
        {
            filePath = String(dirPath) + "/" + file.name();
        }

        if (file.isDirectory())
        {
            if (!deleteAllFilesInDirectory(filePath.c_str()))
            {
                Serial.println(String("Failed to delete directory: ") + filePath);
                return false;
            }
            LittleFS.rmdir(filePath.c_str());
        }
        else
        {
            file.close(); // Close the file if it's open
            if (!LittleFS.remove(filePath.c_str()))
            {
                Serial.println(String("Failed to remove file: ") + filePath);
                return false;
            }
        }
        file = dir.openNextFile();
    }

    return true;
}

void cmdSetup()
{
    hardwareCMD();
    networkCMD();
    sensorCMD();
    zsrelayCMD();
    readConfigCMD();
    commandMap["deleteAll"] = []()
    { deleteAllFilesInLittleFS(); };
    commandMap["ls"] = []()
    { listFilesInDirectory(); };
    commandMap["open"] = []()
    { printFileContent(); };
    commandMap["info"] = []()
    { ESPinfo(); };
    commandMap["net"] = []()
    { networkState(); };
    commandMap["i2cScanner"] = []()
    { i2cScanner(); };
    commandMap["help"] = [&]()
    {
        Serial.println("Available commands:");
        for (const auto &command : commandMap)
        {
            Serial.println(command.first);
        }
    };
}

void beagleCLI()
{
    if (Serial.available() > 0)
    {
        String receivedCommand = Serial.readStringUntil('\n');
        receivedCommand.trim();
        Serial.println("Received command: " + receivedCommand);
        processCommand(receivedCommand);
    }
}

void i2cScanner()
{
    byte error, address;
    int nDevices;

    Serial.println("Scanning...");

    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
        // The i2c_scanner uses the return value of
        // the Write.endTransmission to see if
        // a device did acknowledge to the address.
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            Serial.print("I2C device found at address 0x");
            if (address < 16)
            {
                Serial.print("0");
            }
            Serial.print(address, HEX);
            Serial.println("  !");

            nDevices++;
        }
        else if (error == 4)
        {
            Serial.print("Unknown error at address 0x");
            if (address < 16)
            {
                Serial.print("0");
            }
            Serial.println(address, HEX);
        }
    }
    if (nDevices == 0)
    {
        Serial.println("No I2C devices found\n");
    }
    else
    {
        Serial.println("done\n");
    }

    // Wait 5 seconds for the next scan
}