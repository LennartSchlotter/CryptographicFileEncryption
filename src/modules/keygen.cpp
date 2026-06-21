#include "modules/keygen.h"

#include <argon2.h>
#include <sodium/crypto_aead_aes256gcm.h>
#include <sodium/crypto_aead_chacha20poly1305.h>
#include <sodium/crypto_kdf_hkdf_sha256.h>
#include <sodium/crypto_kem.h>
#include <sodium/crypto_scalarmult.h>

#include "handler.h"
#include <cstring>

namespace crypto {
Result keygen(const KeygenRequest& request) {
    switch (request.algorithm) {
        case CryptoAlgorithms::AES_256_GCM:
        case CryptoAlgorithms::ChaCha20_POLY1305:
        case CryptoAlgorithms::ECDH_X25519:
        case CryptoAlgorithms::ML_KEM_768:
            break;
    }

    return Result{};
}
}  // namespace crypto
