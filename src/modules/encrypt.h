#ifndef ENCRYPT_H

#define ENCRYPT_H

#include "handler.h"

namespace crypto {

// Handles encrypting the file passed in the request.
Result encrypt(const EncryptRequest& request);

// Handles preparing the necessary requirements for symmetric encryption.
std::vector<uint8_t> prepare_symmetric(std::ofstream& result_file);

// Handles preparing the necessary requirements for asymmetric encryption.
std::vector<uint8_t> prepare_asymmetric(std::ofstream& result_file);

// Encrypts the passed file using the provided algorithm and key.
void encrypt(std::ofstream& file, CryptoAlgorithms algorithm, std::vector<uint8_t> key);
}  // namespace crypto

#endif
