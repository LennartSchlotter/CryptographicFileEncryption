
#ifndef DECRYPT_H

#define DECRYPT_H

#include "handler.h"
#include "helper/secure_allocator.h"

namespace crypto {

// Handles decrypting the file passed in the request.
Result decrypt(const DecryptRequest& request);

// Handles preparing the necessary requirements for symmetric decryption.
std::vector<uint8_t, SecureAllocator<uint8_t>> prepare_symmetric(const std::vector<char, SecureAllocator<char>>& encrypted_file_content);

// Handles preparing the necessary requirements for asymmetric decryption.
std::vector<uint8_t, SecureAllocator<uint8_t>> prepare_asymmetric(const std::vector<char, SecureAllocator<char>>& encrypted_file_content, const CryptoAlgorithms& algorithm);

// Decrypts the passed file using the provided algorithm and key.
void decrypt(const std::vector<char, SecureAllocator<char>>& encrypted_file_content, const std::string& output_path, const CryptoAlgorithms& algorithm, const std::vector<unsigned char, SecureAllocator<unsigned char>>& key);
}  // namespace crypto

#endif
