#include "sungrow_client.hpp"
#include "data_converter.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>

struct RegisterScanResult {
    uint16_t address;
    bool success;
    std::vector<uint16_t> values;
    std::string error;
};

class RegisterScanner {
public:
    RegisterScanner(const std::string& host, uint16_t port = 502, uint8_t slaveId = 1)
        : _client(host, port, slaveId) {}
    
    bool connect() {
        return _client.connect();
    }
    
    void disconnect() {
        _client.disconnect();
    }
    
    // Test individual register addresses
    RegisterScanResult scanSingleRegister(uint16_t address) {
        RegisterScanResult result = {address, false, {}, ""};
        
        try {
            auto values = _client.readInputRegisters(address, 1);
            if (!values.empty()) {
                result.success = true;
                result.values = values;
            }
        }
        catch (const std::exception& e) {
            result.error = e.what();
        }
        
        return result;
    }
    
    // Test range of registers
    std::vector<RegisterScanResult> scanRange(uint16_t startAddr, uint16_t count, uint16_t registerSize = 1) {
        std::vector<RegisterScanResult> results;
        
        for (uint16_t addr = startAddr; addr < startAddr + count; addr += registerSize) {
            auto result = scanSingleRegister(addr);
            results.push_back(result);
            
            // Small delay to avoid overwhelming the inverter
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            if (result.success) {
                std::cout << "✓ Register " << addr << " (0x" << std::hex << addr << std::dec 
                         << "): " << result.values[0] << " (0x" << std::hex << result.values[0] << std::dec << ")" << std::endl;
            }
        }
        
        return results;
    }
    
    // Test specific registers likely to contain power data
    void scanPowerRegisters() {
        std::cout << "\n=== SCANNING POWER REGISTERS ===\n" << std::endl;
        
        // Known register areas from various Sungrow documentation
        std::vector<std::pair<uint16_t, uint16_t>> powerRanges = {
            {4990, 20},   // Around device info area
            {5000, 50},   // Core data area  
            {5030, 20},   // Active power area
            {5090, 30},   // Energy consumption area
            {5110, 10},   // Runtime area
            {5140, 10}    // Total yields area
        };
        
        for (auto& range : powerRanges) {
            std::cout << "Scanning range " << range.first << "-" << (range.first + range.second - 1) 
                     << " (0x" << std::hex << range.first << "-0x" << (range.first + range.second - 1) << std::dec << "):" << std::endl;
            
            scanRange(range.first, range.second);
            std::cout << std::endl;
        }
    }
    
    // Test multi-register reads for 32-bit values
    void testMultiRegisterReads(const std::vector<uint16_t>& workingAddresses) {
        std::cout << "\n=== TESTING MULTI-REGISTER READS ===\n" << std::endl;
        
        ModbusDataConverter converter;
        
        for (uint16_t addr : workingAddresses) {
            try {
                auto values = _client.readInputRegisters(addr, 2);
                if (values.size() >= 2) {
                    uint32_t combined = converter.convertU32(values[0], values[1]);
                    double scaled = converter.applyAccuracy(combined, 0.1);
                    
                    std::cout << "Register " << addr << "-" << (addr + 1) << ": " 
                             << values[0] << ", " << values[1] 
                             << " → U32: " << combined << ", Scaled: " << scaled << std::endl;
                }
            }
            catch (const std::exception& e) {
                std::cout << "Multi-read failed for " << addr << ": " << e.what() << std::endl;
            }
        }
    }

private:
    SungrowTcpClient _client;
};

int main(int argc, char* argv[]) {
    std::string host = "192.168.1.249";
    
    if (argc > 1) {
        host = argv[1];
    }
    
    std::cout << "╔══════════════════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                    SG8K-D REGISTER SCANNER                              ║" << std::endl;
    std::cout << "║                  Finding Working Register Addresses                     ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════════════════════════╝" << std::endl;
    
    RegisterScanner scanner(host);
    
    std::cout << "\nConnecting to SG8K-D inverter at " << host << ":502..." << std::endl;
    
    if (!scanner.connect()) {
        std::cerr << "Failed to connect to inverter!" << std::endl;
        return 1;
    }
    
    std::cout << "Connected successfully!" << std::endl;
    
    // Scan for working registers
    scanner.scanPowerRegisters();
    
    // Test some specific registers we know should work
    std::cout << "\n=== TESTING SPECIFIC REGISTER PATTERNS ===\n" << std::endl;
    
    std::vector<uint16_t> testRegisters = {
        4998,  // Device type (zero-based)
        4988,  // Serial start
        5000,  // Basic status
        5002,  // Daily power yields
        5030,  // Active power
        5092   // Export energy
    };
    
    std::vector<uint16_t> workingAddresses;
    
    for (uint16_t addr : testRegisters) {
        auto result = scanner.scanSingleRegister(addr);
        if (result.success) {
            workingAddresses.push_back(addr);
        }
    }
    
    if (!workingAddresses.empty()) {
        scanner.testMultiRegisterReads(workingAddresses);
    }
    
    scanner.disconnect();
    std::cout << "\nScan complete!" << std::endl;
    
    return 0;
}