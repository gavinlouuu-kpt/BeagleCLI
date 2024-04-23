#include <Arduino.h>
#include <beagleCLI.h>

// Define the command as a byte array
uint8_t all_on[] = {0x01, 0x06, 0x00, 0x34, 0x00, 0x01, 0x09, 0xC4};
// '01 06 00 34 00 00 C8 04'
uint8_t all_off[] = {0x01, 0x06, 0x00, 0x34, 0x00, 0x00, 0xC8, 0x04};


void relay_on (){
    Serial2.write(all_on, sizeof(all_on));
}


void relay_off (){
    Serial2.write(all_off, sizeof(all_off));
}


void zsrelayCMD(){
    commandMap["onRelay"] = []() { relay_on(); };
    commandMap["offRelay"] = []() { relay_off(); };
}