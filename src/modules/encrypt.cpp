#include "modules/encrypt.h"

#include <argon2.h>
#include <sodium/crypto_aead_aes256gcm.h>
#include <sodium/crypto_aead_chacha20poly1305.h>
#include <sodium/crypto_box.h>
#include <sodium/crypto_kdf_hkdf_sha256.h>
#include <sodium/crypto_kem.h>
#include <sodium/crypto_kx.h>
#include <sodium/crypto_scalarmult.h>
#include <sodium/crypto_stream_chacha20.h>
#include <sodium/randombytes.h>
#include <sys/types.h>

#include <algorithm>
#include <array>
#include <bit>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "handler.h"
#include "helper/cryptography.h"
#include "helper/password.h"
#include "helper/secure_allocator.h"

namespace crypto {
Result encrypt(const EncryptRequest& request) {
    const std::array<char, 8> file_signature = {
        0x43, 0x46, 0x45, 0x2A, 0x5F, 0x43, 0x4C, 0x49,
    };

    std::vector<uint8_t, SecureAllocator<uint8_t>> key;
    std::vector<unsigned char, SecureAllocator<unsigned char>> header;

    // Write Magic Bytes
    header.insert(header.end(), file_signature.begin(), file_signature.end());

    if (is_asymmetric(request.algorithm)) {
        key = prepare_asymmetric(header, request);
    } else {
        key = prepare_symmetric(header, request);
    }

    encrypt(header, request, key);

    Result result_val{.message = "Successfully encrypted the file", .success = true};
    return result_val;
}

std::vector<uint8_t, SecureAllocator<uint8_t>> prepare_symmetric(
    std::vector<unsigned char, SecureAllocator<unsigned char>>& header,
    const EncryptRequest& request) {
    const size_t SALT_LENGTH = 12;
    const size_t IV_LENGTH = 12;
    const size_t KEY_LENGTH = 32;
    const uint32_t t_cost = 3;
    const uint32_t m_cost = 65536;
    const uint32_t parallelism = 4;

    // Call to `read_password` to retrieve the passphrase
    std::vector<char, SecureAllocator<char>> pw =
        read_password("Please enter the passphrase to use for encryption: ");

    const size_t pw_len = pw.size();

    // Process the password
    std::array<char, SALT_LENGTH> salt{};
    randombytes_buf(salt.data(), salt.size());

    std::vector<uint8_t, SecureAllocator<uint8_t>> derived_key(KEY_LENGTH);

    const int result =
        argon2id_hash_raw(t_cost, m_cost, parallelism, pw.data(), pw_len, salt.data(), salt.size(),
                          derived_key.data(), derived_key.size());

    if (result != ARGON2_OK) {
        unexpected_error(argon2_error_message(result));
    }

    // Write header to the result file
    const char format_version = 1;
    header.emplace_back(format_version);

    const auto algorithm_value = std::to_underlying(request.algorithm);
    const char algorithm_id = static_cast<char>(algorithm_value);
    header.emplace_back(algorithm_id);

    header.insert(header.end(), salt.begin(), salt.end());

    const auto t_cost_char = std::bit_cast<std::array<char, sizeof(uint32_t)>>(t_cost);
    const auto m_cost_char = std::bit_cast<std::array<char, sizeof(uint32_t)>>(m_cost);
    const char parallel_char = static_cast<char>(parallelism);
    header.insert(header.end(), t_cost_char.begin(), t_cost_char.end());
    header.insert(header.end(), m_cost_char.begin(), m_cost_char.end());
    header.emplace_back(parallel_char);

    std::array<char, IV_LENGTH> iv{};
    randombytes_buf(iv.data(), iv.size());
    header.insert(header.end(), iv.begin(), iv.end());

    const uint64_t file_size = std::filesystem::file_size(request.request.file_path);
    auto bytes = std::bit_cast<std::array<char, sizeof(uint64_t)>>(file_size);
    header.insert(header.end(), bytes.begin(), bytes.end());

    return derived_key;
}

std::vector<uint8_t, SecureAllocator<uint8_t>> prepare_asymmetric(
    std::vector<unsigned char, SecureAllocator<unsigned char>>& header,
    const EncryptRequest& request) {
    const size_t KEY_LENGTH = 32;
    const size_t AES_KEY_LENGTH = 32;
    const size_t IV_LENGTH = 12;

    // `read_password` with file path to key
    std::vector<char, SecureAllocator<char>> public_key_path =
        read_password("Please enter the path of the public key of the recipient: ");
    using SecureString = std::basic_string<char, std::char_traits<char>, SecureAllocator<char>>;
    const SecureString file_name(public_key_path.begin(), public_key_path.end());
    std::ifstream pk(file_name);

    std::vector<unsigned char, SecureAllocator<unsigned char>> recipient_pk(
        (std::istreambuf_iterator<char>(pk)), std::istreambuf_iterator<char>());
    std::vector<unsigned char, SecureAllocator<unsigned char>> derived_key(KEY_LENGTH);
    std::vector<unsigned char, SecureAllocator<unsigned char>> ephemeral_pk(
        crypto_box_PUBLICKEYBYTES);
    std::vector<unsigned char, SecureAllocator<unsigned char>> kem_ciphertext(
        crypto_kem_CIPHERTEXTBYTES);

    // Get Session Key and Encrypt it
    switch (request.algorithm) {
        case CryptoAlgorithms::ECDH_X25519: {
            std::vector<unsigned char, SecureAllocator<unsigned char>> ephemeral_sk(
                crypto_box_PUBLICKEYBYTES);
            std::vector<unsigned char, SecureAllocator<unsigned char>> shared_point(
                crypto_scalarmult_BYTES);
            std::vector<unsigned char, SecureAllocator<unsigned char>> prk(
                crypto_kdf_hkdf_sha256_KEYBYTES);
            auto context = std::to_array("file-encryption-key");

            // Generate Ephemeral Keypair
            if (crypto_box_keypair(ephemeral_pk.data(), ephemeral_sk.data()) != 0) {
                unexpected_error("Failed to generate a ephemeral keypair");
            }

            // Calculate shared point
            if (crypto_scalarmult(shared_point.data(), ephemeral_sk.data(), recipient_pk.data()) !=
                0) {
                unexpected_error(
                    "Failed to calculate shared point between the ephemeral secret key and the "
                    "public key of the recipient");
            }

            // Derive key from shared point
            if (crypto_kdf_hkdf_sha256_extract(prk.data(), nullptr, 0, shared_point.data(),
                                               shared_point.size()) != 0) {
                unexpected_error("Failed to create master key");
            }

            if (crypto_kdf_hkdf_sha256_expand(derived_key.data(), AES_KEY_LENGTH, context.data(),
                                              context.size(), prk.data()) != 0) {
                unexpected_error("Failed to derive subkey from master key");
            }

            break;
        }
        case CryptoAlgorithms::ML_KEM_768: {
            if (crypto_kem_enc(kem_ciphertext.data(), derived_key.data(), recipient_pk.data()) !=
                0) {
                unexpected_error("Failed to create ciphertext or secret for the passed public key");
            }
        }
        case CryptoAlgorithms::ChaCha20_POLY1305:
        case CryptoAlgorithms::AES_256_GCM: {
            // This case is not reachable.
            // This is enforced through the request.algorithm being passed as const reference.
            std::unreachable();
            break;
        }
    }

    // Write header to the result file
    // Format version
    const char format_version = 1;
    header.emplace_back(format_version);

    // Algorithm ID
    const auto algorithm_value = std::to_underlying(request.algorithm);
    const char algorithm_id = static_cast<char>(algorithm_value);
    header.emplace_back(algorithm_id);

    // Encrypted Key Length
    int key_length = 0;
    if (request.algorithm == CryptoAlgorithms::ECDH_X25519) {
        key_length = crypto_box_PUBLICKEYBYTES;
    } else if (request.algorithm == CryptoAlgorithms::ML_KEM_768) {
        key_length = crypto_kem_CIPHERTEXTBYTES;
    }

    std::array<char, 4> key_length_str{};
    std::to_chars(key_length_str.data(), std::next(key_length_str.data(), key_length_str.size()),
                  key_length);
    header.insert(header.end(), key_length_str.begin(), key_length_str.end());

    // Encrypted Key
    if (request.algorithm == CryptoAlgorithms::ECDH_X25519) {
        std::ranges::transform(ephemeral_pk, header.begin(), [](unsigned char character) {
            return static_cast<char>(character);
        });
    } else if (request.algorithm == CryptoAlgorithms::ML_KEM_768) {
        std::ranges::transform(kem_ciphertext, header.begin(), [](unsigned char character) {
            return static_cast<char>(character);
        });
    }

    // IV
    std::array<char, IV_LENGTH> iv{};
    randombytes_buf(iv.data(), iv.size());
    header.insert(header.end(), header.begin(), header.end());

    // Ciphertext length
    const uint64_t file_size = std::filesystem::file_size(request.request.file_path);
    header.emplace_back(static_cast<char>(file_size));

    return derived_key;
}

void encrypt(std::vector<unsigned char, SecureAllocator<unsigned char>>& header,
             const EncryptRequest& request,
             const std::vector<uint8_t, SecureAllocator<uint8_t>>& key) {
    const size_t IV_LENGTH = 12;
    const int64_t key_length = retrieve_key_length(request.algorithm);
    const uint8_t SYMMETRIC_IV_INDEX = 35;
    const int64_t ASYMMETRIC_IV_INDEX = 14 + key_length;
    std::vector<unsigned char> iv;

    const uint64_t file_size = std::filesystem::file_size(request.request.file_path);
    std::vector<unsigned char, SecureAllocator<unsigned char>> original_file_content(file_size);
    std::vector<char, SecureAllocator<char>> temp_buffer(file_size);
    std::ifstream source{request.request.file_path, std::ios::binary};

    if (!source.is_open()) {
        unexpected_error("Failed to open file.");
    }

    source.read(temp_buffer.data(), static_cast<std::streamsize>(file_size));
    std::ranges::transform(
        temp_buffer, original_file_content.begin(),
        [](char character) -> unsigned char { return static_cast<unsigned char>(character); });

    size_t anticipated_ciphertext_length = original_file_content.size();
    std::vector<unsigned char, SecureAllocator<unsigned char>> encrypted_file_content;
    encrypted_file_content.reserve(anticipated_ciphertext_length);
    std::vector<char, SecureAllocator<char>> encrypted_file_content_char;
    encrypted_file_content_char.reserve(anticipated_ciphertext_length);
    unsigned long long encrypted_length = 0;  // NOLINT: API requires unsigned long long

    // Encrypt with passed algorithm, if the passed algorithm is asymmetric, default to AES_256
    // This retains a switch case so the addition of more algorithms is easier in the future.
    switch (request.algorithm) {
        case CryptoAlgorithms::ChaCha20_POLY1305: {
            int64_t offset = 0;
            anticipated_ciphertext_length += crypto_aead_chacha20poly1305_ABYTES;
            encrypted_file_content.resize(anticipated_ciphertext_length);
            encrypted_file_content_char.resize(anticipated_ciphertext_length);
            if (is_asymmetric(request.algorithm)) {
                offset = ASYMMETRIC_IV_INDEX;
                auto iv_span = std::span(header.begin() + offset,
                                         header.begin() + offset + IV_LENGTH);
                iv.assign(iv_span.begin(), iv_span.end());
            } else {
                offset = SYMMETRIC_IV_INDEX;
                auto iv_span = std::span(header.begin() + offset,
                                         header.begin() + offset + IV_LENGTH);
                iv.assign(iv_span.begin(), iv_span.end());
            }

            if (crypto_aead_chacha20poly1305_encrypt(
                    encrypted_file_content.data(), &encrypted_length, original_file_content.data(),
                    original_file_content.size(), header.data(), header.size(), nullptr, iv.data(),
                    key.data()) != 0) {
                unexpected_error(
                    "Failed to encrypt data. This may be due to the authentication tag being "
                    "invalid.");
            }
            break;
        }
        // There are only 3 other algorithms.
        // 2 asymmetric ones, where this defaults to AES and AES itself.
        case CryptoAlgorithms::ECDH_X25519:
        case CryptoAlgorithms::ML_KEM_768:
        case CryptoAlgorithms::AES_256_GCM: {
            if (crypto_aead_aes256gcm_is_available() == 0) {
                unexpected_error("AES_256-GCM is not available on this CPU.");
            }

            anticipated_ciphertext_length += crypto_aead_aes256gcm_ABYTES;
            encrypted_file_content.resize(anticipated_ciphertext_length);
            encrypted_file_content_char.resize(anticipated_ciphertext_length);
            int64_t offset = 0;
            if (is_asymmetric(request.algorithm)) {
                offset = ASYMMETRIC_IV_INDEX;
                auto iv_span = std::span(header.begin() + offset,
                                         header.begin() + offset + IV_LENGTH);
                iv.assign(iv_span.begin(), iv_span.end());
            } else {
                offset = SYMMETRIC_IV_INDEX;
                auto iv_span = std::span(header.begin() + offset,
                                         header.begin() + offset + IV_LENGTH);
                iv.assign(iv_span.begin(), iv_span.end());
            }

            if (crypto_aead_aes256gcm_encrypt(
                    encrypted_file_content.data(), &encrypted_length, original_file_content.data(),
                    file_size, header.data(), header.size(), nullptr, iv.data(), key.data()) != 0) {
                unexpected_error(
                    "Failed to encrypt data. This may be due to the authentication tag being "
                    "invalid.");
            }
            break;
        }
    }

    // Create File
    const std::filesystem::path path = request.request.output_path + ".cfe";
    std::filesystem::create_directories(path.parent_path());
    std::ofstream file(path, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!file) {
        unexpected_error("Failed to open file created at output path.");
    }

    std::ranges::transform(encrypted_file_content, encrypted_file_content_char.begin(),
                           [](unsigned char character) { return static_cast<char>(character); });

    std::vector<char, SecureAllocator<char>> header_char(header.size());
    std::ranges::transform(header, header_char.begin(),
                        [](unsigned char character) { return static_cast<char>(character); });

    file.write(header_char.data(), static_cast<std::streamsize>(header.size()));
    file.write(encrypted_file_content_char.data(),
               static_cast<std::streamsize>(encrypted_file_content_char.size()));
}

}  // namespace crypto
