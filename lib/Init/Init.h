#ifndef INIT_h
#define INIT_h

// void FactoryReset();

String readConfigValue(const char *path, const char *jsonPath);
void configInit();
void configIntMod(const char *path, int value);
void writeFile(const char *path, const char *message);

extern String TARGET_GROUP;
extern String USER_ID;
extern int pumpSpeed;
extern String FIREBASE_PROJECT_ID;
extern String USER_EMAIL;
extern String USER_PASSWORD;
extern String API_KEY;
extern String DATABASE_URL;
extern long gmtOffset_sec;
extern int daylightOffset_sec;
extern int FIREBASE_PATH;
extern String STORAGE_BUCKET_ID;


#endif