#include "cryptography.h"

#include <sodium/crypto_aead_aegis256.h>
#include <sodium/crypto_aead_chacha20poly1305.h>
#include <sodium/crypto_box.h>
#include <sodium/crypto_kem.h>
#include <sodium/crypto_kem_mlkem768.h>

#include <utility>

#include "handler.h"

bool is_asymmetric(CryptoAlgorithms algorithm) {
    switch (algorithm) {
        case CryptoAlgorithms::ECDH_X25519:
        case CryptoAlgorithms::ML_KEM_768:
            return true;
            break;
        case CryptoAlgorithms::AEGIS_256:
        case CryptoAlgorithms::ChaCha20_POLY1305:
            return false;
            break;
        default: {
            std::unreachable();
        }
    }
}

int64_t retrieve_key_length(CryptoAlgorithms algorithm) {
    switch (algorithm) {
        case CryptoAlgorithms::ECDH_X25519: {
            return crypto_box_PUBLICKEYBYTES;
        }
        case CryptoAlgorithms::ML_KEM_768: {
            return crypto_kem_mlkem768_CIPHERTEXTBYTES;
        }
        case CryptoAlgorithms::AEGIS_256: {
            return crypto_aead_aegis256_KEYBYTES;
        }
        case CryptoAlgorithms::ChaCha20_POLY1305:
            return crypto_aead_chacha20poly1305_KEYBYTES;
        default: {
            std::unreachable();
        }
    }
}