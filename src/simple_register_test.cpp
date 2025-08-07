#include "sungrow_client.hpp"
#include "data_converter.hpp"
#include <iostream>

int main() {
    std::cout << "=== SIMPLE REGISTER TEST FOR WORKING ADDRESS ===" << std::endl;
    
    SungrowTcpClient client("192.168.1.249", 502, 1);
    
    if (!client.connect()) {
        std::cerr << "Failed to connect!" << std::endl;
        return 1;
    }
    
    ModbusDataConverter converter;
    
    // Based on scanner findings, let's test around 5003 and look for function code 0x02
    std::vector<uint16_t> testAddrs = {5001, 5002, 5003, 5004, 5005};
    
    for (uint16_t addr : testAddrs) {
        std::cout << "\n--- Testing Register " << addr << " ---" << std::endl;
        try {
            auto values = client.readInputRegisters(addr, 1);
            
            // If we got any values, print them
            std::cout << "SUCCESS: Register " << addr << " returned " << values.size() << " values:" << std::endl;
            for (size_t i = 0; i < values.size(); i++) {
                std::cout << "  Value[" << i << "]: " << values[i] 
                         << " (0x" << std::hex << values[i] << std::dec << ")" << std::endl;
            }
            
            // If we got 2 or more values, try combining them as U32
            if (values.size() >= 2) {
                uint32_t combined = converter.convertU32(values[0], values[1]);
                double scaled = converter.applyAccuracy(combined, 0.1);
                std::cout << "  Combined U32: " << combined << std::endl;
                std::cout << "  Scaled (x0.1): " << scaled << std::endl;
            }
            
            break;  // Exit on first success
            
        }
        catch (const std::exception& e) {
            std::cout << "FAILED: Register " << addr << ": " << e.what() << std::endl;
        }
    }
    
    client.disconnect();
    std::cout << "\nTest complete." << std::endl;
    return 0;
}