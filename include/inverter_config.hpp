#pragma once

#include <cstdint>
#include <string>

struct InverterConfig {
    std::string host = "192.168.1.249";
    uint16_t port = 502;
    uint8_t slaveId = 1;
    uint16_t timeoutMs = 10000;
    uint8_t retries = 3;
    uint8_t scanIntervalSec = 30;
    uint8_t level = 1;
};

namespace RegisterAddresses {
    // Using actual register addresses found to be working
    constexpr uint16_t DEVICE_TYPE_ADDR = 4999;
    constexpr uint16_t SERIAL_START_ADDR = 4989;
    constexpr uint16_t SERIAL_LENGTH = 10;
    
    // Working register found by scanner
    constexpr uint16_t DAILY_POWER_YIELDS = 5003;  // Confirmed working
    constexpr uint16_t TOTAL_POWER_YIELDS = 5144;
    constexpr uint16_t INTERNAL_TEMPERATURE = 5008;
    constexpr uint16_t PHASE_A_VOLTAGE = 5019;
    constexpr uint16_t TOTAL_ACTIVE_POWER = 5031;
    constexpr uint16_t WORK_STATE_1 = 5038;
    
    constexpr uint16_t DAILY_EXPORT_ENERGY = 5093;
    constexpr uint16_t TOTAL_EXPORT_ENERGY = 5095;
    constexpr uint16_t DAILY_IMPORT_ENERGY = 5097;
    constexpr uint16_t TOTAL_IMPORT_ENERGY = 5099;
    constexpr uint16_t DAILY_DIRECT_CONSUMPTION = 5101;
    constexpr uint16_t TOTAL_DIRECT_CONSUMPTION = 5103;
    constexpr uint16_t DAILY_RUNNING_TIME = 5113;
    
    constexpr uint16_t YEAR = 4999;
    constexpr uint16_t MONTH = 5000;
    constexpr uint16_t DAY = 5001;
    constexpr uint16_t HOUR = 5002;
    constexpr uint16_t MINUTE = 5003;
    constexpr uint16_t SECOND = 5004;
    constexpr uint16_t START_STOP = 5006;
}

struct RegisterRange {
    uint16_t startAddr;
    uint16_t count;
    uint8_t functionCode;
};

namespace RegisterRanges {
    // Zero-based register ranges for efficient reading
    constexpr RegisterRange CORE_DATA = {4999, 38, 0x04};  // 5000 - 1
    constexpr RegisterRange EXTENDED_DATA = {5038, 61, 0x04};  // 5039 - 1
    constexpr RegisterRange ENERGY_DATA = {5099, 100, 0x04};  // 5100 - 1
    constexpr RegisterRange TIME_CONTROL = {4998, 10, 0x03};  // 4999 - 1
    
    // Basic ranges for power and status data
    constexpr RegisterRange BASIC_STATUS = {4998, 50, 0x04};  // Device type + serial + basic data
    constexpr RegisterRange POWER_DATA = {5002, 36, 0x04};   // Daily/Total power yields + status
    constexpr RegisterRange CONSUMPTION_DATA = {5092, 21, 0x04};  // Export/Import energy data
}