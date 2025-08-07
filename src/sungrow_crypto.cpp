#include "sungrow_crypto.hpp"
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <iostream>
#include <cstring>

struct SungrowCrypto::AESContext {
    EVP_CIPHER_CTX* ctx;
    
    AESContext() : ctx(EVP_CIPHER_CTX_new()) {}
    ~AESContext() { 
        if (ctx) EVP_CIPHER_CTX_free(ctx); 
    }
};

SungrowCrypto::SungrowCrypto() : _encryptionEnabled(false) {
    _aesContext = std::make_unique<AESContext>();
}

SungrowCrypto::~SungrowCrypto() = default;

std::vector<uint8_t> SungrowCrypto::getKeyExchangeCommand() {
    return {0x68, 0x68, 0x00, 0x00, 0x00, 0x06, 0xf7, 0x04, 0x0a, 0xe7, 0x00, 0x08};
}

bool SungrowCrypto::initializeEncryption(const std::vector<uint8_t>& publicKey) {
    if (publicKey.size() < 16) {
        std::cerr << "Invalid public key size: " << publicKey.size() << std::endl;
        return false;
    }
    
    _deriveKey(publicKey);
    
    if (EVP_EncryptInit_ex(_aesContext->ctx, EVP_aes_128_ecb(), nullptr, _aesKey.data(), nullptr) != 1) {
        std::cerr << "Failed to initialize AES encryption" << std::endl;
        return false;
    }
    
    EVP_CIPHER_CTX_set_padding(_aesContext->ctx, 0);
    
    _encryptionEnabled = true;
    
    std::cout << "Sungrow encryption initialized successfully" << std::endl;
    std::cout << "AES Key: ";
    for (auto byte : _aesKey) {
        printf("%02X ", byte);
    }
    std::cout << std::endl;
    
    return true;
}

bool SungrowCrypto::isEncryptionEnabled() const {
    return _encryptionEnabled;
}

void SungrowCrypto::_deriveKey(const std::vector<uint8_t>& publicKey) {
    _aesKey.clear();
    _aesKey.reserve(16);
    
    for (int i = 0; i < 16; i++) {
        _aesKey.push_back(publicKey[i] ^ PRIVATE_KEY[i]);
    }
    
    std::cout << "Key derivation - Public Key: ";
    for (int i = 0; i < 16; i++) {
        printf("%02X ", publicKey[i]);
    }
    std::cout << std::endl;
    
    std::cout << "Key derivation - Private Key: ";
    for (int i = 0; i < 16; i++) {
        printf("%02X ", PRIVATE_KEY[i]);
    }
    std::cout << std::endl;
}

std::vector<uint8_t> SungrowCrypto::encryptFrame(const std::vector<uint8_t>& frame) {
    if (!_encryptionEnabled) {
        return frame;
    }
    
    auto paddedFrame = _addPadding(frame);
    uint8_t paddingLength = paddedFrame.size() - frame.size();
    
    std::vector<uint8_t> encrypted(paddedFrame.size());
    int outLen = 0;
    
    if (EVP_EncryptUpdate(_aesContext->ctx, encrypted.data(), &outLen, paddedFrame.data(), paddedFrame.size()) != 1) {
        std::cerr << "AES encryption failed" << std::endl;
        return frame;
    }
    
    encrypted.resize(outLen);
    
    auto cryptoHeader = _createCryptoHeader(frame.size(), paddingLength);
    
    std::vector<uint8_t> result;
    result.reserve(cryptoHeader.size() + encrypted.size());
    result.insert(result.end(), cryptoHeader.begin(), cryptoHeader.end());
    result.insert(result.end(), encrypted.begin(), encrypted.end());
    
    std::cout << "Frame encrypted: " << frame.size() << " -> " << result.size() << " bytes" << std::endl;
    
    return result;
}

std::vector<uint8_t> SungrowCrypto::decryptFrame(const std::vector<uint8_t>& encryptedFrame) {
    if (!_encryptionEnabled) {
        return encryptedFrame;
    }
    
    std::cout << "Decrypting frame of " << encryptedFrame.size() << " bytes: ";
    for (auto byte : encryptedFrame) {
        printf("0x%02X ", byte);
    }
    std::cout << std::endl;
    
    // For responses, we need to decrypt differently
    // The response format appears to be: [MBAP header][encrypted response]
    // Skip MBAP header (first 6 bytes) and decrypt the rest
    if (encryptedFrame.size() <= 6) {
        std::cout << "Frame too short for decryption" << std::endl;
        return encryptedFrame;
    }
    
    // Extract MBAP header and encrypted payload
    std::vector<uint8_t> mbapHeader(encryptedFrame.begin(), encryptedFrame.begin() + 6);
    std::vector<uint8_t> encryptedPayload(encryptedFrame.begin() + 6, encryptedFrame.end());
    
    // Pad encrypted payload to 16-byte boundary if needed
    while (encryptedPayload.size() % 16 != 0) {
        encryptedPayload.push_back(0x00);
    }
    
    std::cout << "MBAP header: ";
    for (auto byte : mbapHeader) {
        printf("0x%02X ", byte);
    }
    std::cout << std::endl;
    
    std::cout << "Encrypted payload (" << encryptedPayload.size() << " bytes): ";
    for (auto byte : encryptedPayload) {
        printf("0x%02X ", byte);
    }
    std::cout << std::endl;
    
    // Decrypt the payload
    std::vector<uint8_t> decrypted(encryptedPayload.size());
    int outLen = 0;
    
    EVP_CIPHER_CTX* decryptCtx = EVP_CIPHER_CTX_new();
    if (EVP_DecryptInit_ex(decryptCtx, EVP_aes_128_ecb(), nullptr, _aesKey.data(), nullptr) != 1) {
        EVP_CIPHER_CTX_free(decryptCtx);
        std::cerr << "Failed to initialize decryption context" << std::endl;
        return encryptedFrame;
    }
    
    EVP_CIPHER_CTX_set_padding(decryptCtx, 0);
    
    if (EVP_DecryptUpdate(decryptCtx, decrypted.data(), &outLen, encryptedPayload.data(), encryptedPayload.size()) != 1) {
        std::cerr << "AES decryption failed" << std::endl;
        EVP_CIPHER_CTX_free(decryptCtx);
        return encryptedFrame;
    }
    
    EVP_CIPHER_CTX_free(decryptCtx);
    
    std::cout << "Decrypted payload (" << outLen << " bytes): ";
    for (int i = 0; i < outLen; i++) {
        printf("0x%02X ", decrypted[i]);
    }
    std::cout << std::endl;
    
    // Reconstruct the full Modbus frame
    std::vector<uint8_t> result;
    result.insert(result.end(), mbapHeader.begin(), mbapHeader.end());
    result.insert(result.end(), decrypted.begin(), decrypted.begin() + outLen);
    
    // Remove any padding zeros from the end
    while (result.size() > 6 && result.back() == 0x00) {
        result.pop_back();
    }
    
    std::cout << "Final decrypted frame (" << result.size() << " bytes): ";
    for (auto byte : result) {
        printf("0x%02X ", byte);
    }
    std::cout << std::endl;
    
    return result;
}

std::vector<uint8_t> SungrowCrypto::_addPadding(const std::vector<uint8_t>& data) {
    size_t paddingNeeded = 16 - (data.size() % 16);
    if (paddingNeeded == 16) paddingNeeded = 0;
    
    std::vector<uint8_t> padded = data;
    for (size_t i = 0; i < paddingNeeded; i++) {
        padded.push_back(0x00);
    }
    
    return padded;
}

std::vector<uint8_t> SungrowCrypto::_removePadding(const std::vector<uint8_t>& data) {
    return data;
}

std::vector<uint8_t> SungrowCrypto::_createCryptoHeader(uint16_t length, uint8_t paddingLength) {
    std::vector<uint8_t> header(4);
    
    header[0] = (length >> 8) & 0xFF;
    header[1] = length & 0xFF;
    header[2] = 0x00;
    header[3] = paddingLength;
    
    return header;
}

bool SungrowCrypto::_parseCryptoHeader(const std::vector<uint8_t>& data, uint16_t& length, uint8_t& paddingLength) {
    if (data.size() < 4) return false;
    
    length = (static_cast<uint16_t>(data[0]) << 8) | data[1];
    paddingLength = data[3];
    
    return true;
}