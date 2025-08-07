#include "data_converter.hpp"

uint16_t ModbusDataConverter::convertU16(uint16_t rawValue) const {
    if (isInvalidU16(rawValue)) return 0;
    return rawValue;
}

uint32_t ModbusDataConverter::convertU32(uint16_t highWord, uint16_t lowWord) const {
    if (isInvalidU32(highWord, lowWord)) return 0;
    return (static_cast<uint32_t>(highWord) << 16) | lowWord;
}

int16_t ModbusDataConverter::convertS16(uint16_t rawValue) const {
    if (rawValue == 0xFFFF || rawValue == 0x7FFF) return 0;
    return (rawValue >= 32767) ? (rawValue - 65536) : rawValue;
}

std::string ModbusDataConverter::convertUTF8(const std::vector<uint16_t>& registers, size_t start, size_t count) const {
    std::string result;
    for (size_t i = 0; i < count && (start + i) < registers.size(); i++) {
        uint16_t reg = registers[start + i];
        result += static_cast<char>((reg >> 8) & 0xFF);
        result += static_cast<char>(reg & 0xFF);
    }
    result.erase(std::find(result.begin(), result.end(), '\0'), result.end());
    return result;
}

double ModbusDataConverter::applyAccuracy(uint32_t rawValue, double accuracy) const {
    return static_cast<double>(rawValue) * accuracy;
}

bool ModbusDataConverter::isInvalidU16(uint16_t value) const {
    return value == 0xFFFF;
}

bool ModbusDataConverter::isInvalidU32(uint16_t highWord, uint16_t lowWord) const {
    return highWord == 0xFFFF && lowWord == 0xFFFF;
}