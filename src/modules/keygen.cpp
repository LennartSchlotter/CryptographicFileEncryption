#include "modules/keygen.h"

#include <__expected/unexpected.h>
#include <argon2.h>
#include <sodium/crypto_aead_aes256gcm.h>
#include <sodium/crypto_aead_chacha20poly1305.h>
#include <sodium/crypto_box.h>
#include <sodium/crypto_kdf_hkdf_sha256.h>
#include <sodium/crypto_kem.h>
#include <sodium/crypto_kem_mlkem768.h>
#include <sodium/crypto_kx.h>
#include <sodium/crypto_scalarmult.h>

#include "handler.h"
#include <cstring>

namespace crypto {
std::expected<Result, Result> keygen(const KeygenRequest& request) {
    switch (request.algorithm) {
        case CryptoAlgorithms::AEGIS_256:
        case CryptoAlgorithms::ChaCha20_POLY1305: {
            return unexpected_error("Key generation is not supported for asymmetric algorithms");
        }
        case CryptoAlgorithms::ECDH_X25519: {
            std::array<unsigned char, crypto_scalarmult_curve25519_BYTES> x255_pk = {};
            std::array<unsigned char, crypto_scalarmult_curve25519_BYTES> x255_sk = {};

            if (crypto_box_keypair(x255_pk.data(), x255_sk.data()) != 0) {
                return unexpected_error("Failed to generate X25519 keypair");
            }
            break;
        }
        case CryptoAlgorithms::ML_KEM_768: {
            std::array<unsigned char,crypto_kem_mlkem768_PUBLICKEYBYTES> mlkem_pk = {};
            std::array<unsigned char, crypto_kem_mlkem768_SECRETKEYBYTES> mlkem_sk = {};

            if (crypto_kem_mlkem768_keypair(mlkem_pk.data(), mlkem_sk.data()) != 0) {
                return unexpected_error("Failed to generate ML-KEM-768 keypair");
            }
            break;
        }
        default: {
            return unexpected_error("Unsupported algorithm for key generation");
        }
    }

    return Result{.message = "Keypair generated successfully." ,.success = true};
}
}  // namespace crypto
