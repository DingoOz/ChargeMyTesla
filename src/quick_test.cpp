#include "sungrow_client.hpp"
#include "data_converter.hpp"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "=== QUICK REGISTER TEST ===" << std::endl;
    
    SungrowTcpClient client("192.168.1.249", 502, 1);
    
    if (!client.connect()) {
        std::cerr << "Failed to connect!" << std::endl;
        return 1;
    }
    
    ModbusDataConverter converter;
    
    // Test the working register 5003 that was found by scanner
    std::vector<uint16_t> testAddrs = {5003, 5004, 5005, 5006, 5007, 5008, 5009, 5010};
    
    std::cout << "Testing registers around 5003..." << std::endl;
    
    for (uint16_t addr : testAddrs) {
        try {
            auto values = client.readInputRegisters(addr, 1);
            if (!values.empty()) {
                std::cout << "✓ Register " << addr << ": " << values[0] 
                         << " (0x" << std::hex << values[0] << std::dec << ")" << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cout << "✗ Register " << addr << ": " << e.what() << std::endl;
        }
    }
    
    // Test multi-register read on 5003
    std::cout << "\nTesting multi-register read on 5003-5004..." << std::endl;
    try {
        auto values = client.readInputRegisters(5003, 2);
        if (values.size() >= 2) {
            uint32_t combined = converter.convertU32(values[0], values[1]);
            double scaled = converter.applyAccuracy(combined, 0.1);
            std::cout << "Combined U32: " << combined << ", Scaled: " << scaled << " kWh" << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cout << "Multi-register read failed: " << e.what() << std::endl;
    }
    
    client.disconnect();
    return 0;
}