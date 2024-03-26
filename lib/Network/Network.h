#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <WiFi.h>
//state management
void networkState();
void GoogleDNS();
class WiFiManager {
public:
  WiFiManager();
  void ManageWIFI();
  void scanNetworks();
  String selectNetwork();
  String inputPassword();
  bool connectToWiFi(const String& ssid, const String& password);
};


#endif //NETWORK_H