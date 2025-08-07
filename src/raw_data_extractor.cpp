#include "sungrow_client.hpp" 
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "=== RAW DATA PATTERN EXTRACTOR ===" << std::endl;
    std::cout << "Looking for patterns matching phone app values in decrypted data" << std::endl;
    
    SungrowTcpClient client("192.168.1.249", 502, 1);
    
    if (!client.connect()) {
        std::cerr << "Failed to connect!" << std::endl;
        return 1;
    }
    
    std::cout << "\n📱 Target values: Production=16.8, Load=15.1, Feed-in=13.1, Purchased=11.4" << std::endl;
    std::cout << "Looking for: 168, 1680, 151, 1510, 131, 1310, 114, 1140 (scaled versions)" << std::endl;
    
    // Test registers around the area where scanner found some data
    for (uint16_t addr = 5000; addr <= 5010; addr++) {
        try {
            std::cout << "\n--- Register " << addr << " ---" << std::endl;
            
            // Force a raw register read and get the full decrypted frame
            auto frame = client._buildModbusFrame(0x04, addr, 1);  // This won't work - private function
            
            // Instead, let's examine what we can get
            auto values = client.readInputRegisters(addr, 1);
            // This will fail but give us debugging output
        }
        catch (const std::exception& e) {
            std::cout << "Register " << addr << ": " << e.what() << std::endl;
            
            // Extract any numeric patterns from the error output
            // The debugging output shows the decrypted frame
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    client.disconnect();
    return 0;
}