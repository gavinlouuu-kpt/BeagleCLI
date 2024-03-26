#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <string>
#include <vector>

struct SensorData {
private:
    std::string infoString;
    std::vector<float> conVec;
    std::vector<uint32_t> dataVec200, dataVec300, dataVec400; // Updated

public:
    // Constructor
    SensorData(const std::string& info, const std::vector<float>& con, const std::vector<uint32_t>& data200, const std::vector<uint32_t>& data300, const std::vector<uint32_t>& data400);

    // Getters
    const std::string& getInfoString() const;
    const std::vector<float>& getConVec() const;
    const std::vector<uint32_t>& getDataVec200() const; // New
    const std::vector<uint32_t>& getDataVec300() const; // New
    const std::vector<uint32_t>& getDataVec400() const; // New

    // Setters
    void setInfoString(const std::string& info);
    void setConVec(const std::vector<float>& con);
    // Optionally add setters for dataVec200, dataVec300, dataVec400
};

#endif // SENSOR_DATA_H
