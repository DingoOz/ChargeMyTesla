cmake_minimum_required(VERSION 3.20)
project(SolarMonitor CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(include)

add_executable(solar_monitor
    src/main.cpp
    src/sungrow_inverter.cpp
    src/sungrow_client.cpp
    src/sungrow_crypto.cpp
    src/data_converter.cpp
)

add_executable(register_scanner
    src/register_scanner.cpp
    src/sungrow_client.cpp
    src/sungrow_crypto.cpp
    src/data_converter.cpp
)

add_executable(quick_test
    src/quick_test.cpp
    src/sungrow_client.cpp
    src/sungrow_crypto.cpp
    src/data_converter.cpp
)

add_executable(simple_register_test
    src/simple_register_test.cpp
    src/sungrow_client.cpp
    src/sungrow_crypto.cpp
    src/data_converter.cpp
)

add_executable(energy_data_reader
    src/energy_data_reader.cpp
    src/sungrow_client.cpp
    src/sungrow_crypto.cpp
    src/data_converter.cpp
)

add_executable(exact_scanner_test
    src/exact_scanner_test.cpp
    src/sungrow_client.cpp
    src/sungrow_crypto.cpp
    src/data_converter.cpp
)

target_link_libraries(solar_monitor 
    Boost::system
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_link_libraries(register_scanner 
    Boost::system
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_link_libraries(quick_test 
    Boost::system
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_link_libraries(simple_register_test 
    Boost::system
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_link_libraries(energy_data_reader 
    Boost::system
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_link_libraries(exact_scanner_test 
    Boost::system
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)

set_target_properties(solar_monitor PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

set_target_properties(register_scanner PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

set_target_properties(quick_test PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

set_target_properties(simple_register_test PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

set_target_properties(energy_data_reader PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

set_target_properties(exact_scanner_test PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)