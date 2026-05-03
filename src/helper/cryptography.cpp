#include "handler.h"
#include <sodium/crypto_box.h>
#include <sodium/crypto_kem.h>

bool is_asymmetric(CryptoAlgorithms algorithm) { // NOLINT(misc-use-internal-linkage)
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
            unexpected_error("An unexpected error has occured. Please try again or report the bug.");
        }
    }
}

int64_t retrieve_key_length(CryptoAlgorithms algorithm) { // NOLINT(misc-use-internal-linkage)
    const size_t SYMMETRIC_KEY_LENGTH = 32;

    switch (algorithm) {
        case CryptoAlgorithms::ECDH_X25519: {
            return crypto_box_PUBLICKEYBYTES;
        }
        case CryptoAlgorithms::ML_KEM_768: {
            return crypto_kem_CIPHERTEXTBYTES;
        }
        default: {
            return SYMMETRIC_KEY_LENGTH;
        }
    }
}