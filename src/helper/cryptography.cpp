#include <sodium/crypto_aead_aes256gcm.h>
#include <sodium/crypto_aead_chacha20poly1305.h>
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
    constexpr size_t MLKEM768_CIPHERTEXTBYTES = 1088;
    switch (algorithm) {
        case CryptoAlgorithms::ECDH_X25519: {
            return crypto_box_PUBLICKEYBYTES;
        }
        case CryptoAlgorithms::ML_KEM_768: {
            return MLKEM768_CIPHERTEXTBYTES;
        }
        case CryptoAlgorithms::AES_256_GCM: {
            return crypto_aead_aes256gcm_KEYBYTES;
        }
        case CryptoAlgorithms::ChaCha20_POLY1305:
            return crypto_aead_chacha20poly1305_KEYBYTES;
        default: {
            std::unreachable();
        }
    }
}