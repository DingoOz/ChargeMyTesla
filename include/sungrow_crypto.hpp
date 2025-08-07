#pragma once

#include <vector>
#include <cstdint>
#include <memory>

class SungrowCrypto {
public:
    SungrowCrypto();
    ~SungrowCrypto();

    bool initializeEncryption(const std::vector<uint8_t>& publicKey);
    bool isEncryptionEnabled() const;
    
    std::vector<uint8_t> encryptFrame(const std::vector<uint8_t>& frame);
    std::vector<uint8_t> decryptFrame(const std::vector<uint8_t>& encryptedFrame);
    
    static std::vector<uint8_t> getKeyExchangeCommand();

private:
    static constexpr uint8_t PRIVATE_KEY[16] = {
        'G', 'r', 'o', 'w', '#', '0', '*', '2',
        'S', 'u', 'n', '6', '8', 'C', 'b', 'E'
    };
    
    std::vector<uint8_t> _aesKey;
    bool _encryptionEnabled;
    
    struct AESContext;
    std::unique_ptr<AESContext> _aesContext;
    
    void _deriveKey(const std::vector<uint8_t>& publicKey);
    std::vector<uint8_t> _addPadding(const std::vector<uint8_t>& data);
    std::vector<uint8_t> _removePadding(const std::vector<uint8_t>& data);
    std::vector<uint8_t> _createCryptoHeader(uint16_t length, uint8_t paddingLength);
    bool _parseCryptoHeader(const std::vector<uint8_t>& data, uint16_t& length, uint8_t& paddingLength);
};