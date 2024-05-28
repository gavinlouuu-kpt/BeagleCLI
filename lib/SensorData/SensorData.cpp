#include "SensorData.h" // Fixed include directive

SensorData::SensorData(const std::string &info, const std::vector<float> &con, const std::vector<uint32_t> &data150, const std::vector<uint32_t> &data200, const std::vector<uint32_t> &data250, const std::vector<uint32_t> &data300, const std::vector<uint32_t> &data350, const std::vector<uint32_t> &data400, const std::vector<uint32_t> &data450, const std::vector<uint32_t> &data500)
    : infoString(info), conVec(con), dataVec150(data150), dataVec200(data200), dataVec250(data250), dataVec300(data300), dataVec350(data350), dataVec400(data400), dataVec450(data450), dataVec500(data500) {}

const std::string &SensorData::getInfoString() const
{
    return infoString;
}

const std::vector<float> &SensorData::getConVec() const
{
    return conVec;
}

// Implement the new getters for dataVec200, dataVec300, dataVec400
const std::vector<uint32_t> &SensorData::getDataVec150() const
{
    return dataVec150;
}

const std::vector<uint32_t> &SensorData::getDataVec200() const
{
    return dataVec200;
}

const std::vector<uint32_t> &SensorData::getDataVec250() const
{
    return dataVec250;
}

const std::vector<uint32_t> &SensorData::getDataVec300() const
{
    return dataVec300;
}

const std::vector<uint32_t> &SensorData::getDataVec350() const
{
    return dataVec350;
}

const std::vector<uint32_t> &SensorData::getDataVec400() const
{
    return dataVec400;
}

const std::vector<uint32_t> &SensorData::getDataVec450() const
{
    return dataVec450;
}

const std::vector<uint32_t> &SensorData::getDataVec500() const
{
    return dataVec500;
}

const std::vector<uint32_t> &SensorData::getData() const
{
    return data;
}
// Existing setters can remain unchanged. Add new setters if needed.
