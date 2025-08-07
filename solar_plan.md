# Solar Data Collection - C++20 Implementation Plan

## Project Overview

This document provides comprehensive technical specifications for creating a C++20 application to extract data from a Sungrow SG8K-D solar inverter. The application will replicate and extend the functionality of the Python SunGather tool.

**Target Inverter**: SG8K-D at IP 192.168.1.249
**Protocol**: Sungrow proprietary Modbus TCP variant
**Working Configuration**: Successfully tested and validated

## Connection Protocol Analysis

### Network Configuration
- **IP Address**: 192.168.1.249
- **Port**: 502 (Modbus TCP standard)
- **Protocol**: Sungrow proprietary Modbus TCP (NOT standard Modbus)
- **Connection Type**: `sungrow` (uses SungrowModbusTcpClient library internally)
- **Slave ID**: 1 (0x01)
- **Timeout**: 10 seconds
- **Retries**: 3 attempts

### Critical Protocol Difference

**IMPORTANT**: The SG8K-D does **NOT** work with standard Modbus TCP. It requires the Sungrow proprietary protocol implemented in `SungrowModbusTcpClient`. Standard Modbus TCP connection attempts result in "Frame check failed" errors.

**Working Connection Log Example**:
```
Connection: SungrowModbusTcpClient(192.168.1.249:502)
SEND: 0x0 0x1 0x0 0x0 0x0 0x6 0x1 0x4 0x13 0x87 0x0 0x1
RECV: 0x0 0x1 0x0 0x0 0x0 0x5 0x1 0x4 0x2 0x24 0x3
Detected Model: SG8K-D
```

## Required C++ Libraries

### Essential Dependencies
1. **Modbus TCP Library**: Need C++ equivalent of SungrowModbusTcpClient
   - Alternative: Implement Sungrow protocol encryption from scratch
   - Libraries to investigate: libmodbus (may need modification), custom implementation
2. **Network Library**: Asio/Boost.Asio for TCP connections
3. **JSON/Configuration**: nlohmann/json or similar for configuration management
4. **Logging**: spdlog for structured logging
5. **Threading**: std::thread, std::chrono for polling intervals

### Crypto Requirements
The SungrowModbusTcpClient uses encryption. From investigation:
- Uses `pycryptodomex` library in Python
- Likely AES encryption of Modbus frames
- **Critical**: Must reverse-engineer or find C++ equivalent

## Modbus Register Mapping (SG8K-D)

### Device Detection Registers
```cpp
// Device Type Code - Address 4999 (0x1387) - Read Input Register
// Response 0x2403 = "SG8K-D"
uint16_t DEVICE_TYPE_ADDR = 4999;

// Serial Number - Address 4989-4998 (0x137D-0x1386) - UTF-8, 10 registers
uint16_t SERIAL_START_ADDR = 4989;
uint16_t SERIAL_LENGTH = 10;
```

### Essential Energy Data Registers (Level 1)

```cpp
struct EnergyRegisters {
    // Input Registers (Function Code 0x04)
    uint16_t daily_power_yields = 5003;        // kWh, U32, accuracy: 0.1
    uint16_t total_power_yields = 5004;        // kWh, U32, accuracy: 0.1  
    uint16_t internal_temperature = 5008;      // °C, U16, accuracy: 0.1
    uint16_t phase_a_voltage = 5019;          // V, U16, accuracy: 0.1
    uint16_t total_active_power = 5031;       // W, U32
    uint16_t work_state_1 = 5038;             // Enum (Running/Standby/etc)
    
    // Smart Meter Data (SG8K-D specific)
    uint16_t daily_export_energy = 5093;      // kWh, U32, accuracy: 0.1
    uint16_t total_export_energy = 5095;      // kWh, U32, accuracy: 0.1  
    uint16_t daily_import_energy = 5097;      // kWh, U32, accuracy: 0.1
    uint16_t total_import_energy = 5099;      // kWh, U32, accuracy: 0.1
    uint16_t daily_direct_consumption = 5101; // kWh, U32, accuracy: 0.1
    uint16_t total_direct_consumption = 5103; // kWh, U32, accuracy: 0.1
    uint16_t daily_running_time = 5113;       // min, U16

    // Holding Registers (Function Code 0x03) 
    uint16_t year = 4999;   // Current time from inverter
    uint16_t month = 5000;
    uint16_t day = 5001;
    uint16_t hour = 5002;
    uint16_t minute = 5003;
    uint16_t second = 5004;
    uint16_t start_stop = 5006;  // Start/Stop state
};
```

### Register Ranges for Efficient Reading

Based on successful SunGather operation, read these register ranges:

```cpp
struct RegisterRange {
    uint16_t start_addr;
    uint16_t count;
    uint8_t function_code;  // 0x03 = Holding, 0x04 = Input
};

// Optimized ranges from working implementation
RegisterRange ranges[] = {
    {5000, 38, 0x04},   // Input registers: Core data
    {5039, 61, 0x04},   // Input registers: Extended data  
    {5100, 100, 0x04},  // Input registers: Energy data
    {4999, 10, 0x03}    // Holding registers: Time/control
};
```

## Data Type Handling

### Modbus Data Conversion (Critical Implementation Details)

```cpp
class ModbusDataConverter {
public:
    // Handle U16 (16-bit unsigned)
    uint16_t convertU16(uint16_t raw_value) {
        if (raw_value == 0xFFFF) return 0;  // Invalid data marker
        return raw_value;
    }
    
    // Handle U32 (32-bit unsigned, big-endian)
    uint32_t convertU32(uint16_t high_word, uint16_t low_word) {
        if (high_word == 0xFFFF && low_word == 0xFFFF) return 0;
        return (static_cast<uint32_t>(high_word) << 16) | low_word;
    }
    
    // Handle S16 (16-bit signed)
    int16_t convertS16(uint16_t raw_value) {
        if (raw_value == 0xFFFF || raw_value == 0x7FFF) return 0;
        return (raw_value >= 32767) ? (raw_value - 65536) : raw_value;
    }
    
    // Handle UTF-8 strings (Serial number)
    std::string convertUTF8(const std::vector<uint16_t>& registers, size_t start, size_t count) {
        std::string result;
        for (size_t i = 0; i < count; i++) {
            uint16_t reg = registers[start + i];
            result += static_cast<char>((reg >> 8) & 0xFF);  // High byte
            result += static_cast<char>(reg & 0xFF);         // Low byte
        }
        // Remove null terminators
        result.erase(std::find(result.begin(), result.end(), '\0'), result.end());
        return result;
    }
    
    // Apply accuracy scaling
    double applyAccuracy(uint32_t raw_value, double accuracy) {
        return static_cast<double>(raw_value) * accuracy;
    }
};
```

## Sungrow Protocol Implementation Strategy

### Option 1: Reverse Engineer SungrowModbusTcpClient
```cpp
// Key implementation points from Python analysis:
// 1. Uses pycryptodomex for encryption
// 2. Modifies standard Modbus TCP frames
// 3. Implements custom authentication/handshake

class SungrowTcpClient {
private:
    boost::asio::ip::tcp::socket socket_;
    std::string host_;
    uint16_t port_;
    uint8_t slave_id_;
    
    // Encryption keys/methods (TBD - reverse engineering needed)
    bool encrypt_frame(std::vector<uint8_t>& frame);
    bool decrypt_response(std::vector<uint8_t>& response);
    
public:
    bool connect();
    std::vector<uint16_t> read_input_registers(uint16_t address, uint16_t count);
    std::vector<uint16_t> read_holding_registers(uint16_t address, uint16_t count);
    void close();
};
```

### Option 2: Use Python SungrowModbusTcpClient via Embedding
```cpp
// Embed Python and use existing library
#include <Python.h>

class PythonSungrowClient {
    PyObject* client_module_;
    PyObject* client_instance_;
    
public:
    bool initialize();
    std::vector<uint16_t> read_registers(uint16_t address, uint16_t count, bool input_regs = true);
    ~PythonSungrowClient() { Py_Finalize(); }
};
```

## Application Architecture

### Core Classes

```cpp
// Main inverter interface
class SungrowInverter {
private:
    std::unique_ptr<SungrowTcpClient> client_;
    InverterConfig config_;
    RegisterMap registers_;
    DataCache latest_data_;
    
public:
    bool connect();
    bool detect_model();
    bool detect_serial();
    bool scrape_data();
    InverterData get_latest_data() const;
    void disconnect();
};

// Configuration management
struct InverterConfig {
    std::string host = "192.168.1.249";
    uint16_t port = 502;
    uint8_t slave_id = 1;
    uint16_t timeout_ms = 10000;
    uint8_t retries = 3;
    uint8_t scan_interval_sec = 30;
    uint8_t level = 1;  // Data collection depth
};

// Data export interface
class DataExporter {
public:
    virtual void export_data(const InverterData& data) = 0;
};

class ConsoleExporter : public DataExporter {
public:
    void export_data(const InverterData& data) override;
};

class JsonExporter : public DataExporter {
public:
    void export_data(const InverterData& data) override;
};
```

### Main Application Loop

```cpp
int main() {
    try {
        InverterConfig config;
        SungrowInverter inverter(config);
        
        if (!inverter.connect()) {
            throw std::runtime_error("Failed to connect to inverter");
        }
        
        // Main polling loop
        while (true) {
            auto start_time = std::chrono::steady_clock::now();
            
            if (inverter.scrape_data()) {
                auto data = inverter.get_latest_data();
                
                // Export to multiple formats
                ConsoleExporter console;
                console.export_data(data);
                
                JsonExporter json;
                json.export_data(data);
            }
            
            // Sleep until next scan interval
            auto end_time = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
            auto sleep_time = config.scan_interval_sec - duration.count();
            
            if (sleep_time > 0) {
                std::this_thread::sleep_for(std::chrono::seconds(sleep_time));
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
```

## Current Energy Data Fields

Based on successful SunGather output, the application should extract:

```cpp
struct InverterData {
    // Device Info
    std::string device_type = "SG8K-D";
    std::string serial_number = "A211055509";
    std::string run_state = "ON";
    std::string timestamp;
    std::string last_reset;
    
    // Energy Production
    double daily_power_yields = 12.7;     // kWh
    double total_power_yields = 43008.3;   // kWh
    
    // Grid Interaction  
    double daily_export_energy = 6.3;      // kWh
    double total_export_energy = 21257.7;  // kWh
    double daily_import_energy = 20.4;     // kWh  
    double total_import_energy = 23779.5;  // kWh
    
    // Direct Consumption
    double daily_direct_consumption = 6.4;    // kWh
    double total_direct_consumption = 21750.6; // kWh
    
    // System Status
    double internal_temperature = 29.8;    // °C
    double phase_a_voltage = 240.7;        // V
    uint32_t total_active_power = 0;       // W (current generation)
    std::string work_state_1 = "Initial Standby";
    uint16_t daily_running_time = 1320;    // minutes
    
    // Calculated Fields
    uint32_t export_to_grid = 0;           // W (current export)
    uint32_t import_from_grid = 0;         // W (current import)
};
```

## Build System (CMake)

```cmake
cmake_minimum_required(VERSION 3.20)
project(SolarMonitor CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)

# Option: If using Python embedding
# find_package(Python COMPONENTS Interpreter Development REQUIRED)

add_executable(solar_monitor
    src/main.cpp
    src/sungrow_inverter.cpp
    src/sungrow_client.cpp
    src/data_converter.cpp
    src/console_exporter.cpp
    src/json_exporter.cpp
)

target_link_libraries(solar_monitor 
    Boost::system
    spdlog::spdlog
    nlohmann_json::nlohmann_json
)

# Option: If using Python embedding
# target_link_libraries(solar_monitor Python::Python)
```

## Testing Strategy

### Unit Tests
- Modbus frame parsing
- Data type conversions
- Register mapping validation
- Configuration loading

### Integration Tests  
- Live connection to SG8K-D inverter
- Data consistency validation against SunGather output
- Error handling and reconnection logic

### Expected Output Validation
The application should produce output matching this format:
```
+----------------------------------------------------------------------+
| Address | Register                            | Value                |
+---------+-------------------------------------+----------------------+
| 5003    | daily_power_yields                  | 12.7 kWh             |
| 5004    | total_power_yields                  | 43008.3 kWh          |
| 5008    | internal_temperature                | 29.8 °C              |
| 5019    | phase_a_voltage                     | 240.7 V              |
| 5031    | total_active_power                  | 0 W                  |
| 5038    | work_state_1                        | Initial Standby      |
| 5093    | daily_export_energy                 | 6.3 kWh              |
| 5095    | total_export_energy                 | 21257.7 kWh          |
+----------------------------------------------------------------------+
```

## Critical Implementation Notes

1. **Protocol Encryption**: The Sungrow protocol is encrypted. Standard Modbus libraries will NOT work.

2. **Register Addressing**: Modbus addresses in code vs documentation may differ by 1 (0-based vs 1-based indexing).

3. **Data Endianness**: Multi-register values (U32) are big-endian with high word first.

4. **Error Handling**: 0xFFFF values typically indicate invalid/unavailable data.

5. **Timing**: The working implementation includes a 3-second delay after connection.

6. **Connection Management**: Keep connections open between reads for better performance.

## Future Extensions

### Planned Features
- Web dashboard with real-time data
- Database persistence (SQLite, PostgreSQL)
- MQTT publishing for Home Assistant integration
- Historical data analysis and reporting
- Multiple inverter support
- Configuration via web interface

### Performance Optimizations
- Concurrent register reading
- Caching frequently accessed data  
- Configurable polling intervals per register type
- Connection pooling for multiple inverters

## Security Considerations

- Store configuration in secure files with appropriate permissions
- Implement connection encryption if extending beyond local network
- Log security events (connection attempts, failures)
- Consider firewall rules for inverter access

## Documentation Requirements

- API documentation for all public interfaces
- Configuration file format specification
- Deployment and installation guide
- Troubleshooting guide for common connection issues
- Performance tuning guidelines

This comprehensive plan provides all the technical details necessary for implementing a robust C++20 solar inverter monitoring application that replicates and extends the proven SunGather functionality.