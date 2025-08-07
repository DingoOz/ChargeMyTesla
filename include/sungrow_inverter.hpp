#pragma once

#include "sungrow_client.hpp"
#include "data_converter.hpp"
#include "inverter_config.hpp"
#include <memory>
#include <string>

struct InverterData {
    std::string deviceType = "Unknown";
    std::string serialNumber = "Unknown";
    std::string runState = "Unknown";
    std::string timestamp;
    
    double dailyPowerYields = 0.0;
    double totalPowerYields = 0.0;
    
    double dailyExportEnergy = 0.0;
    double totalExportEnergy = 0.0;
    double dailyImportEnergy = 0.0;
    double totalImportEnergy = 0.0;
    
    double dailyDirectConsumption = 0.0;
    double totalDirectConsumption = 0.0;
    
    double internalTemperature = 0.0;
    double phaseAVoltage = 0.0;
    uint32_t totalActivePower = 0;
    std::string workState1 = "Unknown";
    uint16_t dailyRunningTime = 0;
    
    uint32_t exportToGrid = 0;
    uint32_t importFromGrid = 0;
};

class SungrowInverter {
public:
    explicit SungrowInverter(const InverterConfig& config);
    ~SungrowInverter();

    bool connect();
    void disconnect();
    bool isConnected() const;
    
    bool detectModel();
    bool detectSerial();
    bool scrapeData();
    
    const InverterData& getLatestData() const;
    void printPowerConsumptionStatus() const;

private:
    InverterConfig _config;
    std::unique_ptr<SungrowTcpClient> _client;
    ModbusDataConverter _converter;
    InverterData _latestData;
    
    bool _readPowerData();
    bool _readEnergyData();
    bool _readSystemStatus();
    std::string _getWorkStateString(uint16_t stateCode) const;
};