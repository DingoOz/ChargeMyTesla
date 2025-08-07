#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>

class ModbusDataConverter {
public:
    uint16_t convertU16(uint16_t rawValue) const;
    uint32_t convertU32(uint16_t highWord, uint16_t lowWord) const;
    int16_t convertS16(uint16_t rawValue) const;
    std::string convertUTF8(const std::vector<uint16_t>& registers, size_t start, size_t count) const;
    double applyAccuracy(uint32_t rawValue, double accuracy) const;
    
    bool isInvalidU16(uint16_t value) const;
    bool isInvalidU32(uint16_t highWord, uint16_t lowWord) const;
};