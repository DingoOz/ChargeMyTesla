# Sungrow SG8K-D Solar Inverter Monitor

Get real-time data from your Sungrow SG8K-D solar inverter.

## Quick Commands

### Get Latest Data (Recommended)
```bash
cd SunGather/SunGather && ../venv/bin/python3 sungather.py -c ../sg8kd-config.yaml
```

### Build and Run C++ Monitor
```bash
cmake -B build && cmake --build build
./build/solar_monitor
```

## Sample Output

```
╔══════════════════════════════════════════════════════════════════════════════╗
║                              DAILY SUMMARY                                   ║
║  📈 Solar Production:     16.8 kWh  (What your panels generated today)      ║
║  🏠 House Load:          15.5 kWh  (What your house consumed today)         ║
║  ⬆️  Export to Grid:      13.1 kWh  (Sold back to utility)                 ║
║  ⬇️  Import from Grid:    11.8 kWh  (Bought from utility)                  ║
║  💡 Current Status: STANDBY (Night mode, no solar generation)               ║
╚══════════════════════════════════════════════════════════════════════════════╝
```

## Installation

**Ubuntu/Debian:**
```bash
sudo apt install build-essential cmake libboost-system-dev libssl-dev
```

**Python SunGather is already configured and ready to use.**

## Available Tools

| Command | Description |
|---------|-------------|
| `cd SunGather/SunGather && ../venv/bin/python3 sungather.py -c ../sg8kd-config.yaml` | **Get live data** (most reliable) |
| `./build/solar_monitor` | C++ real-time monitor |
| `./build/energy_data_reader` | Energy validation tool |

## Configuration

- **Inverter IP**: 192.168.1.249 (configured in sg8kd-config.yaml)
- **Protocol**: Sungrow encrypted Modbus TCP
- **Port**: 502

---

**Note**: The Python SunGather tool provides the most reliable real-time data. The C++ tools are for development and testing.