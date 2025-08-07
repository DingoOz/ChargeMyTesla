# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a C++20 solar inverter monitoring project specifically designed to interface with Sungrow SG8K-D solar inverters. The project aims to extract energy data from the inverter using Sungrow's proprietary Modbus TCP protocol and replicate the functionality of the Python SunGather tool.

**Key Technologies:**
- C++20 with modern standards
- Sungrow proprietary Modbus TCP protocol (NOT standard Modbus)
- Target inverter: SG8K-D at IP 192.168.1.249
- Network communication via TCP port 502

## Development Commands

Currently, no build system has been implemented. Based on the project plan, the following commands will be needed once implementation begins:

### Build System (Planned)
- `cmake -B build && cmake --build build` - Build the project
- `./build/solar_monitor` - Run the main application

### Dependencies (When Implemented)
Required C++ libraries:
- Boost.Asio for TCP networking
- spdlog for logging  
- nlohmann/json for configuration
- Custom Sungrow protocol implementation (encryption required)

## Code Standards

This project must strictly follow the **Perseus Software Standards** as documented in `Perseus_software_standards.md`. Key requirements include:

### Naming Conventions
- Variables: `camelCase` (e.g., `dailyPowerYields`)
- Classes: `PascalCase` (e.g., `SungrowInverter`)
- Files: `snake_case` (e.g., `sungrow_client.cpp`)
- Constants: `SCREAMING_SNAKE_CASE`
- Private members: prefixed with `_` (e.g., `_client`)

### File Structure
- Source files: `.cpp` extension
- Header files: `.hpp` extension  
- Headers in `include/` directory
- Source files in `src/` directory
- Use `#pragma once` for header guards

### Code Quality
- All functions must be documented with Doxygen comments
- No compiler warnings acceptable
- Prefer const-correctness throughout
- Use RAII for resource management
- Functions should be short and focused (under 40 lines recommended)

## Project Architecture

### Core Components (Planned)

**SungrowInverter**: Main interface class for inverter communication
- Handles connection management to SG8K-D inverter
- Manages data polling and collection
- Implements device detection and serial number reading

**SungrowTcpClient**: Low-level protocol implementation  
- Critical: Uses Sungrow proprietary encryption (NOT standard Modbus)
- Handles TCP connection to inverter at 192.168.1.249:502
- Implements encrypted Modbus frame handling

**DataConverter**: Modbus data type handling
- Converts U16, U32, S16 register values
- Handles big-endian multi-register values
- Processes UTF-8 encoded strings (serial numbers)
- Applies accuracy scaling to raw values

**DataExporter**: Output formatting
- ConsoleExporter for terminal output
- JsonExporter for structured data
- Extensible for future database/MQTT integration

### Key Register Addresses
Critical Modbus registers for SG8K-D inverter:
- Device Type: Address 4999 (0x1387) - Returns 0x2403 for "SG8K-D"  
- Serial Number: Addresses 4989-4998 (UTF-8, 10 registers)
- Daily Power Yields: Address 5003 (kWh, U32, accuracy: 0.1)
- Total Power Yields: Address 5004 (kWh, U32, accuracy: 0.1)
- Energy export/import data: Addresses 5093-5103

## Critical Implementation Notes

### Protocol Requirements
**CRITICAL**: The SG8K-D inverter does NOT support standard Modbus TCP. It requires Sungrow's proprietary encrypted protocol. Standard Modbus connections will fail with "Frame check failed" errors.

Working connection pattern:
```
SEND: 0x0 0x1 0x0 0x0 0x0 0x6 0x1 0x4 0x13 0x87 0x0 0x1
RECV: 0x0 0x1 0x0 0x0 0x0 0x5 0x1 0x4 0x2 0x24 0x3
```

### Data Handling
- Multi-register values (U32) are big-endian with high word first
- 0xFFFF values indicate invalid/unavailable data
- Connection requires 3-second delay after establishment
- Keep connections open between reads for performance

### Current Status
This repository contains only planning documentation. No source code has been implemented yet. The `solar_plan.md` file contains comprehensive technical specifications for the complete implementation.

When implementing, start with the basic TCP connection and device detection before building the full data collection system.