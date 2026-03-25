#ifndef ENCRYPT_H

#define ENCRYPT_H

#include "handler.h"
#include "helper/secure_allocator.h"

namespace crypto {

// Handles encrypting the file passed in the request.
Result encrypt(const EncryptRequest& request);

// Handles preparing the necessary requirements for symmetric encryption.
std::vector<char, SecureAllocator> prepare_symmetric(std::ofstream& result_file);

// Handles preparing the necessary requirements for asymmetric encryption.
std::vector<char, SecureAllocator> prepare_asymmetric(std::ofstream& result_file);

// Encrypts the passed file using the provided algorithm and key.
void encrypt(std::ofstream file, CryptoAlgorithms algorithm, std::vector<char, SecureAllocator> key);
}  // namespace crypto

#endif
