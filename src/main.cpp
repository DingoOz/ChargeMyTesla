#include "sungrow_inverter.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>

std::atomic<bool> running{true};

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ". Shutting down..." << std::endl;
    running = false;
}

void printHeader() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                   SG8K-D SOLAR INVERTER MONITOR                       ║\n";
    std::cout << "║                        Power Consumption Reader                        ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════════╝\n";
    std::cout << std::endl;
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]\n";
    std::cout << "Options:\n";
    std::cout << "  --host <ip>      Inverter IP address (default: 192.168.1.249)\n";
    std::cout << "  --port <port>    Inverter port (default: 502)\n";
    std::cout << "  --interval <sec> Scan interval in seconds (default: 30)\n";
    std::cout << "  --once           Read once and exit\n";
    std::cout << "  --help           Show this help message\n";
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    InverterConfig config;
    bool readOnce = false;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
        else if (arg == "--host" && i + 1 < argc) {
            config.host = argv[++i];
        }
        else if (arg == "--port" && i + 1 < argc) {
            config.port = std::stoi(argv[++i]);
        }
        else if (arg == "--interval" && i + 1 < argc) {
            config.scanIntervalSec = std::stoi(argv[++i]);
        }
        else if (arg == "--once") {
            readOnce = true;
        }
        else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    printHeader();
    
    std::cout << "Connecting to SG8K-D inverter at " << config.host << ":" << config.port << std::endl;
    
    try {
        SungrowInverter inverter(config);
        
        if (!inverter.connect()) {
            std::cerr << "ERROR: Failed to connect to inverter at " << config.host << ":" << config.port << std::endl;
            std::cerr << "Please check:" << std::endl;
            std::cerr << "  - Inverter is powered on and connected to network" << std::endl;
            std::cerr << "  - IP address is correct (" << config.host << ")" << std::endl;
            std::cerr << "  - Port 502 is accessible" << std::endl;
            std::cerr << "  - No firewall blocking the connection" << std::endl;
            return 1;
        }
        
        std::cout << "Connection established successfully!" << std::endl;
        
        std::cout << "\nDetecting inverter model..." << std::endl;
        inverter.detectModel();
        
        std::cout << "Reading serial number..." << std::endl;
        inverter.detectSerial();
        
        if (readOnce) {
            std::cout << "\nReading power consumption data..." << std::endl;
            if (inverter.scrapeData()) {
                inverter.printPowerConsumptionStatus();
            } else {
                std::cerr << "ERROR: Failed to read inverter data" << std::endl;
                return 1;
            }
        }
        else {
            std::cout << "\nStarting continuous monitoring (interval: " << config.scanIntervalSec << " seconds)" << std::endl;
            std::cout << "Press Ctrl+C to stop..." << std::endl;
            
            while (running) {
                auto startTime = std::chrono::steady_clock::now();
                
                std::cout << "\n--- Reading inverter data ---" << std::endl;
                
                if (inverter.scrapeData()) {
                    inverter.printPowerConsumptionStatus();
                } else {
                    std::cerr << "WARNING: Failed to read data from inverter" << std::endl;
                }
                
                if (!running) break;
                
                auto endTime = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
                auto sleepTime = config.scanIntervalSec - duration.count();
                
                if (sleepTime > 0) {
                    std::cout << "\nWaiting " << sleepTime << " seconds until next reading..." << std::endl;
                    for (int i = 0; i < sleepTime && running; i++) {
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                    }
                }
            }
        }
        
        std::cout << "\nDisconnecting from inverter..." << std::endl;
        inverter.disconnect();
        std::cout << "Program terminated successfully." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        std::cerr << "\nTroubleshooting tips:" << std::endl;
        std::cerr << "1. Verify the SG8K-D inverter is accessible at " << config.host << std::endl;
        std::cerr << "2. Check that Modbus TCP is enabled on the inverter" << std::endl;
        std::cerr << "3. Ensure no other software is connected to the inverter" << std::endl;
        std::cerr << "4. Try running with --host <correct_ip> if IP has changed" << std::endl;
        
        return 1;
    }
    
    return 0;
}