#include "sungrow_client.hpp"
#include <iostream>

int main() {
    std::cout << "=== SIMPLE REGISTER TEST ===\n" << std::endl;
    
    InverterConfig config;
    SungrowTcpClient client(config.host, config.port, config.slaveId);
    
    if (client.connect()) {
        std::cout << "Connected successfully!" << std::endl;
        
        try {
            std::cout << "\nTesting register 5000 (device type)..." << std::endl;
            auto regs = client.readInputRegisters(5000, 1);
            std::cout << "Success! Received " << regs.size() << " registers" << std::endl;
            for (auto reg : regs) {
                std::cout << "Register value: 0x" << std::hex << reg << std::dec << " (" << reg << ")" << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Register read failed: " << e.what() << std::endl;
        }
        
        client.disconnect();
    }
    
    return 0;
}