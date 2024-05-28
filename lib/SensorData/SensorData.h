#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <string>
#include <vector>

struct SensorData
{
private:
    std::string infoString;
    std::vector<float> conVec;
    std::vector<uint32_t> data, dataVec150, dataVec200, dataVec250, dataVec300, dataVec350, dataVec400, dataVec450, dataVec500; // Updated

public:
    // Constructor
    // SensorData(const std::string &info, const std::vector<float> &con, const std::vector<uint32_t> &data200, const std::vector<uint32_t> &data300, const std::vector<uint32_t> &data400);
    SensorData(const std::string &info, const std::vector<float> &con, const std::vector<uint32_t> &data150, const std::vector<uint32_t> &data200, const std::vector<uint32_t> &data250, const std::vector<uint32_t> &data300, const std::vector<uint32_t> &data350, const std::vector<uint32_t> &data400, const std::vector<uint32_t> &data450, const std::vector<uint32_t> &data500);

    // Getters
    const std::string &getInfoString() const;
    const std::vector<float> &getConVec() const;
    const std::vector<uint32_t> &getData() const;
    const std::vector<uint32_t> &getDataVec150() const;
    const std::vector<uint32_t> &getDataVec200() const;
    const std::vector<uint32_t> &getDataVec250() const;
    const std::vector<uint32_t> &getDataVec300() const;
    const std::vector<uint32_t> &getDataVec350() const;
    const std::vector<uint32_t> &getDataVec400() const;
    const std::vector<uint32_t> &getDataVec450() const;
    const std::vector<uint32_t> &getDataVec500() const;

    // Setters
    void setInfoString(const std::string &info);
    void setConVec(const std::vector<float> &con);
    // Optionally add setters for dataVec200, dataVec300, dataVec400
};

#endif // SENSOR_DATA_H
