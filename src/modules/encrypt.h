#ifndef ENCRYPT_H

#define ENCRYPT_H

#include <sys/types.h>

#include <cstdint>

#include "handler.h"
#include "helper/secure_allocator.h"

namespace crypto {

// Handles encrypting the file passed in the request.
Result encrypt(const EncryptRequest& request);

// Handles preparing the necessary requirements for symmetric encryption.
std::vector<uint8_t, SecureAllocator<uint8_t>> prepare_symmetric(
    std::vector<unsigned char, SecureAllocator<unsigned char>>& header,
    const EncryptRequest& request);

// Handles preparing the necessary requirements for asymmetric encryption.
std::vector<uint8_t, SecureAllocator<uint8_t>> prepare_asymmetric(
    std::vector<unsigned char, SecureAllocator<unsigned char>>& header,
    const EncryptRequest& request);

// Encrypts the passed file using the provided algorithm and key.
void encrypt(std::vector<unsigned char, SecureAllocator<unsigned char>>& header,
             const EncryptRequest& request,
             const std::vector<uint8_t, SecureAllocator<uint8_t>>& key);
}  // namespace crypto

#endif
