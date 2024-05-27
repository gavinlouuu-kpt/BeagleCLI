#ifndef EXP_SETUP_H
#define EXP_SETUP_H

#include <Arduino.h>

void readConfigCMD();
void read_number_of_setups();
void load_sd_json(const char *filename, String &configData);
void M5_SD_JSON(const char *filename, String &configData);

#endif // EXP_SETUP_H
