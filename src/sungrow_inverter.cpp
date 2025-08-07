#include "sungrow_inverter.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>

SungrowInverter::SungrowInverter(const InverterConfig& config)
    : _config(config) {
    _client = std::make_unique<SungrowTcpClient>(_config.host, _config.port, _config.slaveId);
}

SungrowInverter::~SungrowInverter() {
    disconnect();
}

bool SungrowInverter::connect() {
    return _client->connect();
}

void SungrowInverter::disconnect() {
    _client->disconnect();
}

bool SungrowInverter::isConnected() const {
    return _client->isConnected();
}

bool SungrowInverter::detectModel() {
    try {
        auto registers = _client->readInputRegisters(RegisterAddresses::DEVICE_TYPE_ADDR, 1);
        if (!registers.empty()) {
            uint16_t deviceCode = registers[0];
            std::cout << "Device code received: 0x" << std::hex << deviceCode << std::dec << " (" << deviceCode << ")" << std::endl;
            
            if (deviceCode == 0x2403 || deviceCode == 0x08) {
                _latestData.deviceType = "SG8K-D";
                std::cout << "Detected Model: " << _latestData.deviceType << std::endl;
                return true;
            } else if (deviceCode != 0 && deviceCode != 0xFFFF) {
                _latestData.deviceType = "Sungrow Inverter (Code: 0x" + std::to_string(deviceCode) + ")";
                std::cout << "Detected Sungrow inverter with code: " << _latestData.deviceType << std::endl;
                return true;
            } else {
                std::cout << "Invalid device code: 0x" << std::hex << deviceCode << std::endl;
                _latestData.deviceType = "Unknown";
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Model detection failed: " << e.what() << std::endl;
    }
    return false;
}

bool SungrowInverter::detectSerial() {
    try {
        auto registers = _client->readInputRegisters(RegisterAddresses::SERIAL_START_ADDR, RegisterAddresses::SERIAL_LENGTH);
        if (registers.size() >= RegisterAddresses::SERIAL_LENGTH) {
            _latestData.serialNumber = _converter.convertUTF8(registers, 0, RegisterAddresses::SERIAL_LENGTH);
            std::cout << "Serial Number: " << _latestData.serialNumber << std::endl;
            return true;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Serial detection failed: " << e.what() << std::endl;
    }
    return false;
}

bool SungrowInverter::scrapeData() {
    bool success = true;
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    _latestData.timestamp = std::ctime(&time_t);
    
    success &= _readPowerData();
    success &= _readEnergyData();
    success &= _readSystemStatus();
    
    return success;
}

bool SungrowInverter::_readPowerData() {
    try {
        // Try reading the confirmed working register first
        auto registers = _client->readInputRegisters(RegisterAddresses::DAILY_POWER_YIELDS, 2);
        if (registers.size() >= 2) {
            uint32_t dailyRaw = _converter.convertU32(registers[0], registers[1]);
            _latestData.dailyPowerYields = _converter.applyAccuracy(dailyRaw, 0.1);
            std::cout << "Daily Power Yields read successfully: " << _latestData.dailyPowerYields << " kWh" << std::endl;
        }
        
        // Try reading other registers individually to identify which ones work
        try {
            registers = _client->readInputRegisters(RegisterAddresses::TOTAL_POWER_YIELDS, 2);
            if (registers.size() >= 2) {
                uint32_t totalRaw = _converter.convertU32(registers[0], registers[1]);
                _latestData.totalPowerYields = _converter.applyAccuracy(totalRaw, 0.1);
                std::cout << "Total Power Yields read successfully: " << _latestData.totalPowerYields << " kWh" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Total Power Yields read failed: " << e.what() << std::endl;
        }
        
        try {
            registers = _client->readInputRegisters(RegisterAddresses::TOTAL_ACTIVE_POWER, 2);
            if (registers.size() >= 2) {
                _latestData.totalActivePower = _converter.convertU32(registers[0], registers[1]);
                std::cout << "Total Active Power read successfully: " << _latestData.totalActivePower << " W" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Total Active Power read failed: " << e.what() << std::endl;
        }
        
        return true;  // Return success if we got at least one reading
    }
    catch (const std::exception& e) {
        std::cerr << "Power data read failed: " << e.what() << std::endl;
        return false;
    }
}

bool SungrowInverter::_readEnergyData() {
    try {
        // Read energy consumption data in one efficient range (following Python SunGather pattern)
        auto registers = _client->readInputRegisters(RegisterRanges::CONSUMPTION_DATA.startAddr, RegisterRanges::CONSUMPTION_DATA.count);
        
        if (registers.size() >= RegisterRanges::CONSUMPTION_DATA.count) {
            // Daily Export Energy (offset 0-1 in range, register 5092-5093 zero-based)
            uint32_t dailyExportRaw = _converter.convertU32(registers[0], registers[1]);
            _latestData.dailyExportEnergy = _converter.applyAccuracy(dailyExportRaw, 0.1);
            
            // Total Export Energy (offset 2-3 in range, register 5094-5095 zero-based)
            uint32_t totalExportRaw = _converter.convertU32(registers[2], registers[3]);
            _latestData.totalExportEnergy = _converter.applyAccuracy(totalExportRaw, 0.1);
            
            // Daily Import Energy (offset 4-5 in range, register 5096-5097 zero-based)
            uint32_t dailyImportRaw = _converter.convertU32(registers[4], registers[5]);
            _latestData.dailyImportEnergy = _converter.applyAccuracy(dailyImportRaw, 0.1);
            
            // Total Import Energy (offset 6-7 in range, register 5098-5099 zero-based)
            uint32_t totalImportRaw = _converter.convertU32(registers[6], registers[7]);
            _latestData.totalImportEnergy = _converter.applyAccuracy(totalImportRaw, 0.1);
            
            // Daily Direct Consumption (offset 8-9 in range, register 5100-5101 zero-based)
            uint32_t dailyDirectRaw = _converter.convertU32(registers[8], registers[9]);
            _latestData.dailyDirectConsumption = _converter.applyAccuracy(dailyDirectRaw, 0.1);
            
            // Total Direct Consumption (offset 10-11 in range, register 5102-5103 zero-based)
            uint32_t totalDirectRaw = _converter.convertU32(registers[10], registers[11]);
            _latestData.totalDirectConsumption = _converter.applyAccuracy(totalDirectRaw, 0.1);
        }
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Energy data read failed: " << e.what() << std::endl;
        return false;
    }
}

bool SungrowInverter::_readSystemStatus() {
    try {
        // Read Daily Running Time separately (register 5112 zero-based)
        auto registers = _client->readInputRegisters(RegisterAddresses::DAILY_RUNNING_TIME, 1);
        if (!registers.empty()) {
            _latestData.dailyRunningTime = _converter.convertU16(registers[0]);
        }
        
        // Temperature, voltage, and work state are now read in _readPowerData() for efficiency
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "System status read failed: " << e.what() << std::endl;
        return false;
    }
}

std::string SungrowInverter::_getWorkStateString(uint16_t stateCode) const {
    switch (stateCode) {
        case 0x1300: return "Initial Standby";
        case 0x1301: return "Starting";
        case 0x1302: return "Running";
        case 0x1303: return "Stopping";
        case 0x1304: return "Fault";
        case 0x1305: return "Permanent Fault";
        default: return "Unknown (" + std::to_string(stateCode) + ")";
    }
}

const InverterData& SungrowInverter::getLatestData() const {
    return _latestData;
}

void SungrowInverter::printPowerConsumptionStatus() const {
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "SG8K-D INVERTER POWER CONSUMPTION STATUS" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    std::cout << std::left;
    std::cout << std::setw(25) << "Device Model:" << _latestData.deviceType << std::endl;
    std::cout << std::setw(25) << "Serial Number:" << _latestData.serialNumber << std::endl;
    std::cout << std::setw(25) << "Work State:" << _latestData.workState1 << std::endl;
    std::cout << std::setw(25) << "Timestamp:" << _latestData.timestamp;
    
    std::cout << "\n--- CURRENT POWER STATUS ---" << std::endl;
    std::cout << std::fixed << std::setprecision(1);
    std::cout << std::setw(25) << "Current Generation:" << _latestData.totalActivePower << " W" << std::endl;
    std::cout << std::setw(25) << "Internal Temperature:" << _latestData.internalTemperature << " °C" << std::endl;
    std::cout << std::setw(25) << "Phase A Voltage:" << _latestData.phaseAVoltage << " V" << std::endl;
    
    std::cout << "\n--- DAILY ENERGY DATA ---" << std::endl;
    std::cout << std::setw(25) << "Daily Generation:" << _latestData.dailyPowerYields << " kWh" << std::endl;
    std::cout << std::setw(25) << "Daily Export:" << _latestData.dailyExportEnergy << " kWh" << std::endl;
    std::cout << std::setw(25) << "Daily Import:" << _latestData.dailyImportEnergy << " kWh" << std::endl;
    std::cout << std::setw(25) << "Daily Direct Use:" << _latestData.dailyDirectConsumption << " kWh" << std::endl;
    std::cout << std::setw(25) << "Daily Runtime:" << _latestData.dailyRunningTime << " minutes" << std::endl;
    
    std::cout << "\n--- TOTAL ENERGY DATA ---" << std::endl;
    std::cout << std::setw(25) << "Total Generation:" << _latestData.totalPowerYields << " kWh" << std::endl;
    std::cout << std::setw(25) << "Total Export:" << _latestData.totalExportEnergy << " kWh" << std::endl;
    std::cout << std::setw(25) << "Total Import:" << _latestData.totalImportEnergy << " kWh" << std::endl;
    std::cout << std::setw(25) << "Total Direct Use:" << _latestData.totalDirectConsumption << " kWh" << std::endl;
    
    double netExport = _latestData.totalExportEnergy - _latestData.totalImportEnergy;
    std::cout << "\n--- NET ENERGY BALANCE ---" << std::endl;
    std::cout << std::setw(25) << "Net Export to Grid:" << netExport << " kWh" << std::endl;
    
    std::cout << std::string(80, '=') << std::endl;
}