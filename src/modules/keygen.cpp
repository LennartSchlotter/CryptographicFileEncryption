#include "modules/keygen.h"

#include <__expected/unexpected.h>
#include <argon2.h>
#include <fstream>
#include <ios>
#include <sodium/crypto_aead_aes256gcm.h>
#include <sodium/crypto_aead_chacha20poly1305.h>
#include <sodium/crypto_box.h>
#include <sodium/crypto_kdf_hkdf_sha256.h>
#include <sodium/crypto_kem.h>
#include <sodium/crypto_kem_mlkem768.h>
#include <sodium/crypto_kx.h>
#include <sodium/crypto_scalarmult.h>
#include <vector>

#include "handler.h"
#include <cstring>

namespace crypto {
std::expected<Result, Result> keygen(const KeygenRequest& request) {

    std::vector<char> public_key = {};
    std::vector<char> secret_key = {};

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

            public_key.assign(x255_pk.begin(), x255_pk.end());
            secret_key.assign(x255_sk.begin(), x255_sk.end());
            break;
        }
        case CryptoAlgorithms::ML_KEM_768: {
            std::array<unsigned char,crypto_kem_mlkem768_PUBLICKEYBYTES> mlkem_pk = {};
            std::array<unsigned char, crypto_kem_mlkem768_SECRETKEYBYTES> mlkem_sk = {};

            if (crypto_kem_mlkem768_keypair(mlkem_pk.data(), mlkem_sk.data()) != 0) {
                return unexpected_error("Failed to generate ML-KEM-768 keypair");
            }

            public_key.assign(mlkem_pk.begin(), mlkem_pk.end());
            secret_key.assign(mlkem_sk.begin(), mlkem_sk.end());
            break;
        }
        default: {
            return unexpected_error("Unsupported algorithm for key generation");
        }
    }

    const std::string pub_path = request.request.output_path + "/key" + ".pub.bin";
    const std::string priv_path = request.request.output_path + "/key" + ".priv.bin";

    std::ofstream pub_file(pub_path, std::ios::binary);
    if (!pub_file.is_open()) {
        return unexpected_error("Failed to create public key file: " + pub_path);
    }
    pub_file.write(public_key.data(), static_cast<std::streamsize>(public_key.size()));
    pub_file.close();

    std::ofstream priv_file(priv_path, std::ios::binary);
    if (!priv_file.is_open()) {
        return unexpected_error("Failed to create private key file: " + priv_path);
    }
    priv_file.write(secret_key.data(), static_cast<std::streamsize>(secret_key.size()));
    priv_file.close();

    return Result{.message = "Keypair generated successfully." ,.success = true};
}
}  // namespace crypto
