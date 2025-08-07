#include "sungrow_client.hpp"
#include "data_converter.hpp"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "=== EXACT SCANNER REPLICATION TEST ===" << std::endl;
    std::cout << "Replicating the exact approach that found register 5003 = 20050" << std::endl;
    
    SungrowTcpClient client("192.168.1.249", 502, 1);
    
    if (!client.connect()) {
        std::cerr << "Failed to connect!" << std::endl;
        return 1;
    }
    
    ModbusDataConverter converter;
    
    // Test a wider range around where the scanner found success
    std::vector<uint16_t> testAddrs = {4995, 4996, 4997, 4998, 4999, 5000, 5001, 5002, 5003, 5004, 5005, 5006, 5007, 5008, 5009, 5010};
    
    std::cout << "\nPhone app target values:" << std::endl;
    std::cout << "Production: 16.8 kWh, Load: 15.1 kWh, Feed-in: 13.1 kWh, Purchased: 11.4 kWh" << std::endl;
    std::cout << "\nLooking for raw values around: 168, 151, 131, 114 (x10) or 1680, 1510, 1310, 1140 (x100)" << std::endl;
    
    for (uint16_t addr : testAddrs) {
        std::cout << "\n--- Testing Register " << addr << " ---" << std::endl;
        try {
            auto values = client.readInputRegisters(addr, 1);
            
            if (!values.empty()) {
                uint16_t rawValue = values[0];
                std::cout << "✓ SUCCESS: Raw value = " << rawValue << " (0x" << std::hex << rawValue << std::dec << ")" << std::endl;
                
                // Test different interpretations
                std::cout << "  Interpretations:" << std::endl;
                std::cout << "    As-is: " << rawValue << std::endl;
                std::cout << "    ÷10:   " << (rawValue / 10.0) << std::endl;
                std::cout << "    ÷100:  " << (rawValue / 100.0) << std::endl;
                std::cout << "    ×0.1:  " << (rawValue * 0.1) << std::endl;
                std::cout << "    ×0.01: " << (rawValue * 0.01) << std::endl;
                
                // Check for matches
                std::vector<std::pair<double, std::string>> targets = {
                    {16.8, "Production"}, {15.1, "Load"}, {13.1, "Feed-in"}, {11.4, "Purchased"}
                };
                
                for (const auto& target : targets) {
                    double tolerance = target.first * 0.1; // 10% tolerance
                    
                    if (abs(rawValue - target.first) <= tolerance) {
                        std::cout << "    🎯 RAW VALUE MATCHES " << target.second << "!" << std::endl;
                    }
                    if (abs((rawValue / 10.0) - target.first) <= tolerance) {
                        std::cout << "    🎯 ÷10 MATCHES " << target.second << "!" << std::endl;
                    }
                    if (abs((rawValue / 100.0) - target.first) <= tolerance) {
                        std::cout << "    🎯 ÷100 MATCHES " << target.second << "!" << std::endl;
                    }
                    if (abs((rawValue * 0.1) - target.first) <= tolerance) {
                        std::cout << "    🎯 ×0.1 MATCHES " << target.second << "!" << std::endl;
                    }
                    if (abs((rawValue * 0.01) - target.first) <= tolerance) {
                        std::cout << "    🎯 ×0.01 MATCHES " << target.second << "!" << std::endl;
                    }
                }
                
                // Don't stop on first success - test all registers
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            
        }
        catch (const std::exception& e) {
            // Ignore individual failures and continue scanning
            std::cout << "❌ Register " << addr << ": Failed" << std::endl;
        }
    }
    
    client.disconnect();
    std::cout << "\n=== TEST COMPLETE ===" << std::endl;
    return 0;
}