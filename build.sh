#!/bin/bash

echo "Building SG8K-D Solar Monitor..."

if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

cd build

echo "Running CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed!"
    echo "Please ensure you have the following dependencies installed:"
    echo "  - cmake (>= 3.20)"
    echo "  - g++ with C++20 support"
    echo "  - libboost-system-dev"
    echo ""
    echo "On Ubuntu/Debian: sudo apt install cmake g++ libboost-system-dev"
    echo "On CentOS/RHEL: sudo yum install cmake gcc-c++ boost-devel"
    exit 1
fi

echo "Building project..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo ""
    echo "Build completed successfully!"
    echo ""
    echo "To run the program:"
    echo "  ./solar_monitor --once          (read once and exit)"
    echo "  ./solar_monitor                 (continuous monitoring)"
    echo "  ./solar_monitor --host 192.168.1.249 --interval 60"
    echo ""
else
    echo "ERROR: Build failed!"
    exit 1
fi