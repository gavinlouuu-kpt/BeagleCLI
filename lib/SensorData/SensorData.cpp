#include "SensorData.h" // Fixed include directive

SensorData::SensorData(const std::string& info, const std::vector<float>& con, const std::vector<uint32_t>& data200, const std::vector<uint32_t>& data300, const std::vector<uint32_t>& data400)
    : infoString(info), conVec(con), dataVec200(data200), dataVec300(data300), dataVec400(data400) {}

const std::string& SensorData::getInfoString() const {
    return infoString;
}

const std::vector<float>& SensorData::getConVec() const {
    return conVec;
}

// Implement the new getters for dataVec200, dataVec300, dataVec400
const std::vector<uint32_t>& SensorData::getDataVec200() const {
    return dataVec200;
}

const std::vector<uint32_t>& SensorData::getDataVec300() const {
    return dataVec300;
}

const std::vector<uint32_t>& SensorData::getDataVec400() const {
    return dataVec400;
}

// Existing setters can remain unchanged. Add new setters if needed.
