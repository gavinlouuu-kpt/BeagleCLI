#ifndef BEAGLECLI_H
#define BEAGLECLI_H

#include <Arduino.h>

void cmdSetup();
void beagleCLI();
String processCommand(const String& receivedCommand); 
void printFileContent();
void printHexFileContent();

void listFilesInDirectory(const String& directoryPath = "/");
bool deleteAllFilesInLittleFS();
bool deleteAllFilesInDirectory(const char *dirPath);
void i2cScanner();
void batRead();

#endif // SAVEDATA_H