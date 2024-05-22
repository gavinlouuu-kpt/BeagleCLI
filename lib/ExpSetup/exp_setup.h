#ifndef EXP_SETUP_H
#define EXP_SETUP_H

#include <Arduino.h>

void readConfigCMD();
void load_sd_json(const char *filename, String configData);
void M5_SD_CMD(const char *filename);

#endif // EXP_SETUP_H
