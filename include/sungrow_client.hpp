#pragma once

#include <boost/asio.hpp>
#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include <chrono>
#include "sungrow_crypto.hpp"

class SungrowTcpClient {
public:
    SungrowTcpClient(const std::string& host, uint16_t port, uint8_t slaveId);
    ~SungrowTcpClient();

    bool connect();
    void disconnect();
    bool isConnected() const;
    
    bool performKeyExchange();

    std::vector<uint16_t> readInputRegisters(uint16_t address, uint16_t count);
    std::vector<uint16_t> readHoldingRegisters(uint16_t address, uint16_t count);

private:
    std::string _host;
    uint16_t _port;
    uint8_t _slaveId;
    
    boost::asio::io_context _ioContext;
    std::unique_ptr<boost::asio::ip::tcp::socket> _socket;
    std::unique_ptr<SungrowCrypto> _crypto;
    bool _connected;
    uint16_t _transactionId;
    
    std::vector<uint8_t> _buildModbusFrame(uint8_t functionCode, uint16_t address, uint16_t count);
    std::vector<uint16_t> _parseModbusResponse(const std::vector<uint8_t>& response);
    bool _sendFrame(const std::vector<uint8_t>& frame);
    std::vector<uint8_t> _receiveResponse();
    
    void _applySungrowEncryption(std::vector<uint8_t>& frame);
    void _removeSungrowEncryption(std::vector<uint8_t>& response);
};