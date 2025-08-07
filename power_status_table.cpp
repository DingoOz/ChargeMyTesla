#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>

struct PowerData {
    std::string category;
    std::string parameter;
    std::string value;
    std::string unit;
    int registerAddr;
};

void printTableSeparator(const std::vector<int>& widths) {
    std::cout << "+";
    for (size_t i = 0; i < widths.size(); i++) {
        std::cout << std::string(widths[i] + 2, '-') << "+";
    }
    std::cout << std::endl;
}

void printTableRow(const std::vector<std::string>& columns, const std::vector<int>& widths) {
    std::cout << "|";
    for (size_t i = 0; i < columns.size() && i < widths.size(); i++) {
        std::cout << " " << std::left << std::setw(widths[i]) << columns[i] << " |";
    }
    std::cout << std::endl;
}

std::string formatValue(double value, const std::string& unit) {
    std::stringstream ss;
    if (unit.find("kWh") != std::string::npos || unit.find("kW") != std::string::npos) {
        ss << std::fixed << std::setprecision(1) << value;
    } else if (unit.find("W") != std::string::npos && unit != "kW") {
        ss << std::fixed << std::setprecision(0) << value;
    } else if (unit.find("V") != std::string::npos) {
        ss << std::fixed << std::setprecision(1) << value;
    } else if (unit.find("A") != std::string::npos) {
        ss << std::fixed << std::setprecision(2) << value;
    } else {
        ss << std::fixed << std::setprecision(1) << value;
    }
    return ss.str() + " " + unit;
}

int main() {
    // Data from SunGather output - organized by category
    std::vector<PowerData> powerData = {
        // === DAILY ENERGY SUMMARY ===
        {"DAILY ENERGY", "Production Today", "16.8", "kWh", 5003},
        {"DAILY ENERGY", "Export to Grid", "13.1", "kWh", 5093},
        {"DAILY ENERGY", "Import from Grid", "11.8", "kWh", 5097},
        {"DAILY ENERGY", "Direct Consumption", "3.7", "kWh", 5101},
        {"DAILY ENERGY", "Load Consumption", "15.5", "kWh", -1}, // Calculated
        
        // === CURRENT POWER STATUS ===
        {"CURRENT POWER", "Total Active Power", "0", "W", 5031},
        {"CURRENT POWER", "Load Power", "746", "W", 5091},
        {"CURRENT POWER", "Meter Power", "746", "W", 5083},
        {"CURRENT POWER", "Export to Grid", "0", "W", -2}, // Virtual reg
        {"CURRENT POWER", "Import from Grid", "746", "W", -3}, // Virtual reg
        
        // === SOLAR GENERATION ===
        {"SOLAR GENERATION", "MPPT 1 Voltage", "0.0", "V", 5011},
        {"SOLAR GENERATION", "MPPT 1 Current", "0.0", "A", 5012},
        {"SOLAR GENERATION", "MPPT 2 Voltage", "0.0", "V", 5013},
        {"SOLAR GENERATION", "MPPT 2 Current", "0.0", "A", 5014},
        {"SOLAR GENERATION", "PV Power Today", "2", "W", 6100},
        {"SOLAR GENERATION", "Daily PV Yields", "8.4", "kWh", 6196},
        
        // === GRID CONNECTION ===
        {"GRID STATUS", "Phase A Voltage", "244.4", "V", 5019},
        {"GRID STATUS", "Phase A Current", "3.05", "A", 5022},
        {"GRID STATUS", "Grid Frequency", "50.01", "Hz", 5035},
        {"GRID STATUS", "Work State", "Standby", "", 5081},
        {"GRID STATUS", "Run State", "Stop", "", 4999},
        
        // === LIFETIME TOTALS ===
        {"LIFETIME TOTALS", "Total Power Yields", "43025.1", "kWh", 5004},
        {"LIFETIME TOTALS", "Total Export Energy", "21270.8", "kWh", 5095},
        {"LIFETIME TOTALS", "Total Import Energy", "23792.5", "kWh", 5099},
        {"LIFETIME TOTALS", "Total Direct Consumption", "21754.3", "kWh", 5103},
        {"LIFETIME TOTALS", "Daily Running Time", "1305", "min", 5113},
        
        // === SYSTEM STATUS ===
        {"SYSTEM STATUS", "Internal Temperature", "21.7", "°C", 5030},
        {"SYSTEM STATUS", "Insulation Resistance", "993", "k-ohm", 5071},
        {"SYSTEM STATUS", "Power Limitation", "100.0", "%", 5008},
        {"SYSTEM STATUS", "Export Limitation", "62.1", "%", 5015},
        {"SYSTEM STATUS", "Inverter Serial", "A211055509", "", 4989}
    };
    
    // Table column widths
    std::vector<int> widths = {16, 25, 12, 8, 4};
    std::vector<std::string> headers = {"CATEGORY", "PARAMETER", "VALUE", "UNIT", "REG"};
    
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                         SUNGROW SG8K-D INVERTER STATUS                       ║\n";
    std::cout << "║                            IP: 192.168.1.249                                ║\n";
    std::cout << "║                        Serial: A211055509                                   ║\n";
    std::cout << "║                      Timestamp: 2025-08-07 21:42:13                         ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
    
    // Print main table
    printTableSeparator(widths);
    printTableRow(headers, widths);
    printTableSeparator(widths);
    
    std::string currentCategory = "";
    for (const auto& data : powerData) {
        if (data.category != currentCategory) {
            if (!currentCategory.empty()) {
                printTableSeparator(widths);
            }
            currentCategory = data.category;
        }
        
        std::vector<std::string> row = {
            data.category,
            data.parameter,
            data.value,
            data.unit,
            data.registerAddr >= 0 ? std::to_string(data.registerAddr) : "CALC"
        };
        printTableRow(row, widths);
    }
    printTableSeparator(widths);
    
    // Summary calculations
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                              DAILY SUMMARY                                   ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════════════════════╣\n";
    std::cout << "║  📈 Solar Production:     16.8 kWh  (What your panels generated today)      ║\n";
    std::cout << "║  🏠 House Load:          15.5 kWh  (What your house consumed today)         ║\n";
    std::cout << "║  ⬆️  Export to Grid:      13.1 kWh  (Sold back to utility)                 ║\n";
    std::cout << "║  ⬇️  Import from Grid:    11.8 kWh  (Bought from utility)                  ║\n";
    std::cout << "║  🔋 Direct Usage:         3.7 kWh  (Used directly from solar)               ║\n";
    std::cout << "║                                                                              ║\n";
    std::cout << "║  💡 Current Status: STANDBY (Night mode, no solar generation)               ║\n";
    std::cout << "║  ⚡ Live Load:       746 W   (House currently using 746 watts)             ║\n";
    std::cout << "║  🌡️  Temperature:     21.7°C (Inverter internal temperature)               ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════════════════╝\n";
    
    // Energy flow explanation
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                            ENERGY FLOW ANALYSIS                             ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════════════════════════╣\n";
    std::cout << "║  Energy Balance Check:                                                       ║\n";
    std::cout << "║    Production (16.8) = Export (13.1) + Direct Use (3.7) ✓                  ║\n";
    std::cout << "║                                                                              ║\n";
    std::cout << "║  Total House Load Calculation:                                              ║\n";
    std::cout << "║    Load = Direct Use + Import = 3.7 + 11.8 = 15.5 kWh ✓                   ║\n";
    std::cout << "║                                                                              ║\n";
    std::cout << "║  Net Grid Usage:                                                             ║\n";
    std::cout << "║    Export - Import = 13.1 - 11.8 = +1.3 kWh (Net seller today) ✓          ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════════════════╝\n";
    
    std::cout << "\nData source: SunGather v0.5.2 with Sungrow encrypted protocol\n";
    std::cout << "Register addresses shown are Modbus input register numbers (zero-based)\n\n";
    
    return 0;
}