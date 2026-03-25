
#ifndef DECRYPT_H

#define DECRYPT_H

#include "handler.h"
#include "helper/secure_allocator.h"

namespace crypto {

// Handles decrypting the file passed in the request.
Result decrypt(const DecryptRequest& request);

// Handles preparing the necessary requirements for symmetric decryption.
std::vector<char, SecureAllocator> prepare_symmetric();

// Handles preparing the necessary requirements for asymmetric decryption.
std::vector<char, SecureAllocator> prepare_asymmetric();

// Decrypts the passed file using the provided algorithm and key.
void decrypt(std::ofstream file, CryptoAlgorithms algorithm, std::vector<char, SecureAllocator> key);
}  // namespace crypto

#endif
