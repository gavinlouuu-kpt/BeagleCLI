#ifndef BEAGLECLI_H
#define BEAGLECLI_H

#include <Arduino.h>
#include <map>
#include <functional>

using CommandHandler = std::function<void(int)>;

extern std::map<String, std::function<void()>> commandMap;
extern std::map<String, CommandHandler> commandHandlers;

String processCommand(const String& receivedCommand); 
void registerCommand(const String& command, CommandHandler handler);

void cmdSetup();
void beagleCLI();

void printFileContent();
void printHexFileContent();

void listFilesInDirectory(const String& directoryPath = "/");
bool deleteAllFilesInLittleFS();
bool deleteAllFilesInDirectory(const char *dirPath);
void i2cScanner();

#endif // 