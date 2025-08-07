#include "sungrow_client.hpp"
#include "data_converter.hpp"
#include <iostream>
#include <iomanip>

struct EnergyData {
    double production = 0.0;
    double load = 0.0;
    double feedIn = 0.0;
    double purchased = 0.0;
    std::string source = "";
};

int main() {
    std::cout << "╔══════════════════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                    SG8K-D ENERGY DATA VALIDATION                        ║" << std::endl;
    std::cout << "║                    Comparing with Phone App Values                      ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════════════════════════╝" << std::endl;
    
    std::cout << "\n📱 **PHONE APP VALUES (Target):**" << std::endl;
    std::cout << "  Production: 16.8 kWh" << std::endl;
    std::cout << "  Load:       15.1 kWh" << std::endl;
    std::cout << "  Feed-in:    13.1 kWh" << std::endl;
    std::cout << "  Purchased:  11.4 kWh" << std::endl;
    
    SungrowTcpClient client("192.168.1.249", 502, 1);
    
    if (!client.connect()) {
        std::cerr << "Failed to connect!" << std::endl;
        return 1;
    }
    
    ModbusDataConverter converter;
    
    // Test likely energy registers with different scaling factors
    struct RegisterTest {
        uint16_t address;
        std::string name;
        std::vector<double> scales;  // Different scaling factors to try
    };
    
    std::vector<RegisterTest> energyRegisters = {
        // Daily energy registers
        {5003, "Daily Power Yields", {1.0, 0.1, 0.01}},
        {5093, "Daily Export Energy", {1.0, 0.1, 0.01}},
        {5095, "Total Export Energy", {1.0, 0.1, 0.01}},
        {5097, "Daily Import Energy", {1.0, 0.1, 0.01}},
        {5099, "Total Import Energy", {1.0, 0.1, 0.01}},
        {5101, "Daily Direct Consumption", {1.0, 0.1, 0.01}},
        {5103, "Total Direct Consumption", {1.0, 0.1, 0.01}},
        
        // Try some other ranges found by scanner
        {5000, "Register 5000", {1.0, 0.1, 0.01}},
        {5001, "Register 5001", {1.0, 0.1, 0.01}},
        {5002, "Register 5002", {1.0, 0.1, 0.01}},
        {5004, "Register 5004", {1.0, 0.1, 0.01}},
        {5005, "Register 5005", {1.0, 0.1, 0.01}},
    };
    
    std::cout << "\n🔍 **TESTING ENERGY REGISTERS:**" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    
    std::vector<EnergyData> candidates;
    
    for (const auto& reg : energyRegisters) {
        try {
            auto values = client.readInputRegisters(reg.address, 2);
            if (values.size() >= 1) {
                uint16_t rawValue = values[0];
                uint32_t combinedValue = 0;
                
                if (values.size() >= 2) {
                    combinedValue = converter.convertU32(values[0], values[1]);
                }
                
                std::cout << "\n📊 Register " << reg.address << " (" << reg.name << "):" << std::endl;
                std::cout << "  Raw 16-bit: " << rawValue << " (0x" << std::hex << rawValue << std::dec << ")" << std::endl;
                
                if (values.size() >= 2) {
                    std::cout << "  Raw 32-bit: " << combinedValue << " (0x" << std::hex << combinedValue << std::dec << ")" << std::endl;
                }
                
                // Test different scaling factors
                for (double scale : reg.scales) {
                    double scaled16 = rawValue * scale;
                    double scaled32 = combinedValue * scale;
                    
                    std::cout << "    Scale x" << scale << ": " << scaled16;
                    if (values.size() >= 2) {
                        std::cout << " (16-bit) / " << scaled32 << " (32-bit)";
                    }
                    
                    // Check if this matches any target value (within 5% tolerance)
                    std::vector<std::pair<double, std::string>> targets = {
                        {16.8, "Production"}, {15.1, "Load"}, {13.1, "Feed-in"}, {11.4, "Purchased"}
                    };
                    
                    for (const auto& target : targets) {
                        double tolerance = target.first * 0.05; // 5% tolerance
                        if (abs(scaled16 - target.first) <= tolerance) {
                            std::cout << " ⭐ MATCHES " << target.second << "!";
                        }
                        if (values.size() >= 2 && abs(scaled32 - target.first) <= tolerance) {
                            std::cout << " ⭐⭐ 32-bit MATCHES " << target.second << "!";
                        }
                    }
                    std::cout << std::endl;
                }
            }
            
            // Small delay between reads
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            
        }
        catch (const std::exception& e) {
            std::cout << "❌ Register " << reg.address << " (" << reg.name << "): " << e.what() << std::endl;
        }
    }
    
    client.disconnect();
    
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "🎯 **SUMMARY:** Look for ⭐ matches above to identify the correct registers!" << std::endl;
    
    return 0;
}