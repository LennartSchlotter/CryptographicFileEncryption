#include <sodium/crypto_box.h>
#include <sodium/crypto_kem.h>
#include <utility>

#include "handler.h"

bool is_asymmetric(CryptoAlgorithms algorithm) {
    switch (algorithm) {
        case CryptoAlgorithms::ECDH_X25519:
        case CryptoAlgorithms::ML_KEM_768:
            return true;
            break;
        case CryptoAlgorithms::AES_256_GCM:
        case CryptoAlgorithms::ChaCha20_POLY1305:
            return false;
            break;
        default: {
            std::unreachable();
        }
    }
}

int64_t retrieve_key_length(CryptoAlgorithms algorithm) {
    const size_t SYMMETRIC_KEY_LENGTH = 32;

    switch (algorithm) {
        case CryptoAlgorithms::ECDH_X25519: {
            return crypto_box_PUBLICKEYBYTES;
        }
        case CryptoAlgorithms::ML_KEM_768: {
            return crypto_kem_CIPHERTEXTBYTES;
        }
        case CryptoAlgorithms::AES_256_GCM:
        case CryptoAlgorithms::ChaCha20_POLY1305:
            return SYMMETRIC_KEY_LENGTH;
        default: {
            std::unreachable();
        }
    }
}