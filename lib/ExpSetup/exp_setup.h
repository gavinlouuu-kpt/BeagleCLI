#ifndef EXP_SETUP_H
#define EXP_SETUP_H

#include <Arduino.h>
#include <string>
#include <vector>
#include <unordered_map>

void readConfigCMD();
// void read_number_of_setups();
// void load_sd_json(const char *filename, String &configData);
void M5_SD_JSON(const char *filename, String &configData);
void sampleTask();
void expMutexSetup();
// class SensorData
// {
// private:
//     std::string infoString;
//     std::vector<float> conVec;
//     std::unordered_map<uint32_t, std::vector<uint32_t>> sensorData;

// public:
//     // Constructor
//     SensorData(const std::string &info, const std::vector<float> &con, const std::unordered_map<uint32_t, std::vector<uint32_t>> &data)
//         : infoString(info), conVec(con), sensorData(data) {}

//     // Info string accessor
//     const std::string &getInfoString() const
//     {
//         return infoString;
//     }

//     // Concentration vector accessor
//     const std::vector<float> &getConVec() const
//     {
//         return conVec;
//     }

//     // Generic data accessor
//     const std::vector<uint32_t> &getDataVec(uint32_t sensorId) const
//     {
//         auto it = sensorData.find(sensorId);
//         if (it != sensorData.end())
//         {
//             return it->second;
//         }
//         // Handle the error or return a static empty vector to avoid exceptions
//         static std::vector<uint32_t> empty;
//         return empty;
//     }
// };

#endif // EXP_SETUP_H
