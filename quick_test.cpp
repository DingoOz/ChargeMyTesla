#include <iostream>
#include "sungrow_client.hpp"
#include "inverter_config.hpp"

int main() {
    std::cout << "=== QUICK REGISTER SCAN ===\n" << std::endl;
    
    InverterConfig config;
    SungrowTcpClient client(config.host, config.port, config.slaveId);
    
    if (!client.connect()) {
        std::cerr << "Connection failed!" << std::endl;
        return 1;
    }
    
    // Try some different register addresses based on research
    std::vector<uint16_t> testRegisters = {
        4999,  // Original device type (from docs)
        5000,  // Updated device type
        5003,  // Daily power yields  
        5019,  // Phase A voltage
        5031,  // Total active power
        5038,  // Work state
        5093,  // Daily export energy
    };
    
    for (auto reg : testRegisters) {
        std::cout << "\n--- Testing register " << reg << " ---" << std::endl;
        try {
            auto result = client.readInputRegisters(reg, 1);
            if (!result.empty()) {
                std::cout << "SUCCESS: Register " << reg << " = 0x" << std::hex << result[0] << std::dec << " (" << result[0] << ")" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "FAILED: " << e.what() << std::endl;
        }
    }
    
    client.disconnect();
    return 0;
}