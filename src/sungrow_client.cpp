#include "sungrow_client.hpp"
#include <iostream>
#include <thread>
#include <stdexcept>

using boost::asio::ip::tcp;

SungrowTcpClient::SungrowTcpClient(const std::string& host, uint16_t port, uint8_t slaveId)
    : _host(host), _port(port), _slaveId(slaveId), _connected(false), _transactionId(0) {
    _crypto = std::make_unique<SungrowCrypto>();
}

SungrowTcpClient::~SungrowTcpClient() {
    disconnect();
}

bool SungrowTcpClient::connect() {
    try {
        _socket = std::make_unique<tcp::socket>(_ioContext);
        
        tcp::resolver resolver(_ioContext);
        auto endpoints = resolver.resolve(_host, std::to_string(_port));
        
        boost::asio::connect(*_socket, endpoints);
        
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        _connected = true;
        std::cout << "Connected to Sungrow inverter at " << _host << ":" << _port << std::endl;
        
        if (performKeyExchange()) {
            std::cout << "Sungrow encryption protocol initialized" << std::endl;
        } else {
            std::cout << "Key exchange failed - falling back to standard Modbus" << std::endl;
        }
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Connection failed: " << e.what() << std::endl;
        _connected = false;
        return false;
    }
}

void SungrowTcpClient::disconnect() {
    if (_socket && _socket->is_open()) {
        _socket->close();
    }
    _connected = false;
}

bool SungrowTcpClient::isConnected() const {
    return _connected && _socket && _socket->is_open();
}

std::vector<uint16_t> SungrowTcpClient::readInputRegisters(uint16_t address, uint16_t count) {
    if (!isConnected()) {
        throw std::runtime_error("Not connected to inverter");
    }
    
    auto frame = _buildModbusFrame(0x04, address, count);
    
    if (!_sendFrame(frame)) {
        throw std::runtime_error("Failed to send Modbus frame");
    }
    
    auto response = _receiveResponse();
    return _parseModbusResponse(response);
}

std::vector<uint16_t> SungrowTcpClient::readHoldingRegisters(uint16_t address, uint16_t count) {
    if (!isConnected()) {
        throw std::runtime_error("Not connected to inverter");
    }
    
    auto frame = _buildModbusFrame(0x03, address, count);
    
    if (!_sendFrame(frame)) {
        throw std::runtime_error("Failed to send Modbus frame");
    }
    
    auto response = _receiveResponse();
    return _parseModbusResponse(response);
}

std::vector<uint8_t> SungrowTcpClient::_buildModbusFrame(uint8_t functionCode, uint16_t address, uint16_t count) {
    std::vector<uint8_t> frame;
    
    ++_transactionId;
    frame.push_back((_transactionId >> 8) & 0xFF);
    frame.push_back(_transactionId & 0xFF);
    
    frame.push_back(0x00);
    frame.push_back(0x00);
    
    frame.push_back(0x00);
    frame.push_back(0x06);
    
    frame.push_back(_slaveId);
    frame.push_back(functionCode);
    
    frame.push_back((address >> 8) & 0xFF);
    frame.push_back(address & 0xFF);
    
    frame.push_back((count >> 8) & 0xFF);
    frame.push_back(count & 0xFF);
    
    _applySungrowEncryption(frame);
    
    return frame;
}

bool SungrowTcpClient::_sendFrame(const std::vector<uint8_t>& frame) {
    try {
        boost::asio::write(*_socket, boost::asio::buffer(frame));
        
        std::cout << "SEND: ";
        for (auto byte : frame) {
            printf("0x%X ", byte);
        }
        std::cout << std::endl;
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Send failed: " << e.what() << std::endl;
        return false;
    }
}

std::vector<uint8_t> SungrowTcpClient::_receiveResponse() {
    std::vector<uint8_t> response(256);
    
    try {
        size_t length = _socket->read_some(boost::asio::buffer(response));
        response.resize(length);
        
        std::cout << "RECV: ";
        for (auto byte : response) {
            printf("0x%X ", byte);
        }
        std::cout << std::endl;
        
        _removeSungrowEncryption(response);
        
        return response;
    }
    catch (const std::exception& e) {
        std::cerr << "Receive failed: " << e.what() << std::endl;
        return {};
    }
}

std::vector<uint16_t> SungrowTcpClient::_parseModbusResponse(const std::vector<uint8_t>& response) {
    if (response.size() < 9) {
        throw std::runtime_error("Response too short");
    }
    
    std::cout << "Response analysis - Size: " << response.size() << " bytes" << std::endl;
    std::cout << "Raw response: ";
    for (size_t i = 0; i < response.size(); i++) {
        printf("0x%02X ", response[i]);
    }
    std::cout << std::endl;
    
    uint8_t functionCode = response[7];
    uint8_t byteCount = response[8];
    
    std::cout << "Function Code: 0x" << std::hex << (int)functionCode << std::dec << std::endl;
    std::cout << "Byte Count/Error: " << (int)byteCount << std::endl;
    
    // Check for error response (function code + 0x80)
    if (functionCode & 0x80) {
        uint8_t originalFunction = functionCode & 0x7F;
        uint8_t errorCode = byteCount;
        
        std::string errorMsg = "Modbus Error - Function: 0x" + std::to_string(originalFunction) + ", Error Code: " + std::to_string(errorCode);
        
        switch (errorCode) {
            case 1: errorMsg += " (Illegal Function)"; break;
            case 2: errorMsg += " (Illegal Data Address)"; break;
            case 3: errorMsg += " (Illegal Data Value)"; break;
            case 4: errorMsg += " (Server Device Failure)"; break;
            default: errorMsg += " (Unknown Error)"; break;
        }
        
        throw std::runtime_error(errorMsg);
    }
    
    // Accept responses with reasonable function codes and byte counts
    if ((functionCode == 0x02 || functionCode == 0x04) && byteCount > 0 && byteCount <= 250) {
        if (response.size() >= 9 + byteCount) {
            // This looks like a valid response
            std::cout << "Valid response detected with " << (int)byteCount << " bytes of data" << std::endl;
        } else {
            throw std::runtime_error("Incomplete response data");
        }
    } else if (byteCount > 250 || byteCount == 0) {
        // This might be an invalid response
        throw std::runtime_error("Invalid response data");
    } else {
        std::cout << "Warning: Unexpected function code 0x" << std::hex << (int)functionCode 
                  << std::dec << ", attempting to parse anyway..." << std::endl;
        if (response.size() < 9 + byteCount) {
            throw std::runtime_error("Incomplete response data");
        }
    }
    
    std::vector<uint16_t> registers;
    for (int i = 0; i < byteCount; i += 2) {
        uint16_t value = (response[9 + i] << 8) | response[9 + i + 1];
        registers.push_back(value);
    }
    
    return registers;
}

bool SungrowTcpClient::performKeyExchange() {
    try {
        std::cout << "Performing Sungrow key exchange..." << std::endl;
        
        auto keyCmd = SungrowCrypto::getKeyExchangeCommand();
        
        std::cout << "Sending key exchange command..." << std::endl;
        boost::asio::write(*_socket, boost::asio::buffer(keyCmd));
        
        std::cout << "KEY_CMD: ";
        for (auto byte : keyCmd) {
            printf("0x%02X ", byte);
        }
        std::cout << std::endl;
        
        std::vector<uint8_t> keyResponse(256);
        size_t keyRespLen = _socket->read_some(boost::asio::buffer(keyResponse));
        keyResponse.resize(keyRespLen);
        
        std::cout << "KEY_RESP: ";
        for (auto byte : keyResponse) {
            printf("0x%02X ", byte);
        }
        std::cout << std::endl;
        
        if (keyResponse.size() >= 25) {
            std::vector<uint8_t> publicKey(keyResponse.end() - 16, keyResponse.end());
            
            std::cout << "Extracted public key: ";
            for (auto byte : publicKey) {
                printf("0x%02X ", byte);
            }
            std::cout << std::endl;
            
            return _crypto->initializeEncryption(publicKey);
        } else {
            std::cerr << "Invalid key exchange response length: " << keyResponse.size() << std::endl;
            return false;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Key exchange failed: " << e.what() << std::endl;
        return false;
    }
}

void SungrowTcpClient::_applySungrowEncryption(std::vector<uint8_t>& frame) {
    if (_crypto && _crypto->isEncryptionEnabled()) {
        frame = _crypto->encryptFrame(frame);
    } else {
        std::cout << "WARNING: Using standard Modbus (encryption not available)" << std::endl;
    }
}

void SungrowTcpClient::_removeSungrowEncryption(std::vector<uint8_t>& response) {
    if (_crypto && _crypto->isEncryptionEnabled()) {
        response = _crypto->decryptFrame(response);
    }
}