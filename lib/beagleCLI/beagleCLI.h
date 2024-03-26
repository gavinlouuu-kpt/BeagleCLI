#ifndef BEAGLECLI_H
#define BEAGLECLI_H

#include <Arduino.h>

extern std::map<String, std::function<void()>> commandMap;
String processCommand(const String& receivedCommand); 


void cmdSetup();
void beagleCLI();

void printFileContent();
void printHexFileContent();

void listFilesInDirectory(const String& directoryPath = "/");
bool deleteAllFilesInLittleFS();
bool deleteAllFilesInDirectory(const char *dirPath);
void i2cScanner();

#endif // SAVEDATA_H