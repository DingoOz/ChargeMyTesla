# Sungrow SG8K-D Solar Inverter Monitor

A C++20 project for monitoring Sungrow SG8K-D solar inverters using their proprietary encrypted Modbus TCP protocol. This project provides real-time access to solar production, energy consumption, and grid interaction data with a beautiful ASCII table display.

## 🌟 Features

- **Real-time Power Monitoring**: Track current solar generation, house load, and grid interaction
- **Daily Energy Summary**: View today's production, consumption, export, and import totals  
- **Comprehensive Status Display**: Monitor inverter health, temperature, and operational state
- **Beautiful ASCII Table Output**: Clear, formatted display of all inverter data
- **Sungrow Protocol Support**: Uses proprietary encrypted communication (not standard Modbus)
- **Multiple Monitoring Tools**: From simple status checks to detailed register scanning

## 📊 Sample Output

```
╔══════════════════════════════════════════════════════════════════════════════╗
║                         SUNGROW SG8K-D INVERTER STATUS                       ║
║                            IP: 192.168.1.249                                ║
║                        Serial: A211055509                                   ║
╚══════════════════════════════════════════════════════════════════════════════╝

+------------------+---------------------------+--------------+----------+------+
| CATEGORY         | PARAMETER                 | VALUE        | UNIT     | REG  |
+------------------+---------------------------+--------------+----------+------+
| DAILY ENERGY     | Production Today          | 16.8         | kWh      | 5003 |
| DAILY ENERGY     | Export to Grid            | 13.1         | kWh      | 5093 |
| DAILY ENERGY     | Import from Grid          | 11.8         | kWh      | 5097 |
| DAILY ENERGY     | Load Consumption          | 15.5         | kWh      | CALC |
+------------------+---------------------------+--------------+----------+------+

╔══════════════════════════════════════════════════════════════════════════════╗
║                              DAILY SUMMARY                                   ║
║  📈 Solar Production:     16.8 kWh  (What your panels generated today)      ║
║  🏠 House Load:          15.5 kWh  (What your house consumed today)         ║
║  ⬆️  Export to Grid:      13.1 kWh  (Sold back to utility)                 ║
║  ⬇️  Import from Grid:    11.8 kWh  (Bought from utility)                  ║
║  💡 Current Status: STANDBY (Night mode, no solar generation)               ║
╚══════════════════════════════════════════════════════════════════════════════╝
```

## 🚀 Quick Start

### Prerequisites

- **C++20 compiler** (GCC 10+ or Clang 12+)
- **CMake 3.20+**
- **System Libraries**: Boost, OpenSSL, pthread

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake libboost-system-dev libssl-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install gcc-c++ cmake boost-devel openssl-devel
```

### Build Instructions

1. **Clone the repository:**
```bash
git clone <your-repo-url>
cd ChargeMyTesla
```

2. **Build the project:**
```bash
cmake -B build && cmake --build build
```

3. **Configure your inverter IP:**
Edit the IP address in the source files or use the default `192.168.1.249`

4. **Run the power status display:**
```bash
./power_status_table
```

## 🛠️ Available Tools

### Main Applications

| Tool | Description | Usage |
|------|-------------|-------|
| `power_status_table` | Beautiful ASCII display of all power data | `./power_status_table` |
| `solar_monitor` | Main inverter monitoring application | `./build/solar_monitor` |
| `register_scanner` | Scan and test register addresses | `./build/register_scanner` |

### Diagnostic Tools

| Tool | Description | Purpose |
|------|-------------|---------|
| `energy_data_reader` | Validate energy readings against known values | Testing |
| `quick_test` | Basic connectivity test | Debugging |
| `simple_register_test` | Test specific register reads | Development |

## 🔧 Configuration

### Inverter Settings
- **IP Address**: Default `192.168.1.249` (modify in source files)
- **Port**: 502 (Modbus TCP)
- **Protocol**: Sungrow proprietary encrypted Modbus
- **Slave ID**: 0x01

### Supported Models
- **Primary**: SG8K-D (tested and verified)
- **Potential**: Other Sungrow inverters with similar protocol

## 📡 Protocol Details

This project uses **Sungrow's proprietary encrypted Modbus TCP protocol**, not standard Modbus. Key features:

- **AES ECB Encryption**: Uses private key `"Grow#0*2Sun68CbE"`
- **Custom Frame Format**: Modified Modbus frames with encryption
- **Register Addressing**: Zero-based addressing (different from documentation)
- **Function Codes**: Primarily 0x04 (Read Input Registers)

### Key Register Addresses

| Register | Parameter | Description |
|----------|-----------|-------------|
| 5003 | `daily_power_yields` | Daily solar production (kWh) |
| 5093 | `daily_export_energy` | Daily grid export (kWh) |
| 5097 | `daily_import_energy` | Daily grid import (kWh) |
| 5101 | `daily_direct_energy_consumption` | Direct solar usage (kWh) |
| 5091 | `load_power` | Current house load (W) |
| 5031 | `total_active_power` | Current solar generation (W) |

## 🐍 Python Alternative

For comparison and validation, a Python SunGather configuration is included:

```bash
# Setup Python environment
cd SunGather
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt

# Run SunGather
cd SunGather
../venv/bin/python3 sungather.py -c ../sg8kd-config.yaml
```

## 📊 Energy Calculations

The system calculates total house consumption as:
```
Load Consumption = Direct Solar Use + Grid Import
Load Consumption = daily_direct_energy_consumption + daily_import_energy
```

Energy balance verification:
```
Solar Production = Grid Export + Direct Solar Use
```

## 🔍 Troubleshooting

### Common Issues

**Connection Failed:**
- Verify inverter IP address is correct
- Ensure inverter is powered on and network accessible
- Check firewall settings on port 502

**"Frame check failed" errors:**
- This is expected for standard Modbus - use Sungrow protocol
- Verify you're using the encrypted client, not standard Modbus

**Build Errors:**
- Ensure C++20 compiler support
- Check all dependencies are installed
- Try `cmake -B build && cmake --build build` to rebuild

### Debug Mode

Enable debug output by modifying source files or using diagnostic tools:
```bash
./build/register_scanner  # Shows detailed protocol communication
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Follow the Perseus Software Standards (see `Perseus_software_standards.md`)
4. Test with your inverter model
5. Submit a pull request

## ⚡ Performance

- **Connection Time**: ~3 seconds initial setup
- **Data Retrieval**: <1 second for all registers
- **Memory Usage**: <10MB typical
- **Update Frequency**: Configurable, recommend 30 seconds minimum

## 🔗 References

- [SunGather Python Implementation](https://github.com/bohdan-s/SunGather)
- [Sungrow Modbus Protocol Documentation](https://github.com/bohdan-s/SunGather/blob/main/SunGather/registers-sungrow.yaml)
- [Perseus Software Standards](Perseus_software_standards.md)

---

**Note**: This project successfully implements the Sungrow encrypted protocol and has been tested with SG8K-D inverters. The beautiful ASCII table display provides comprehensive energy monitoring with validated data matching inverter phone app readings.