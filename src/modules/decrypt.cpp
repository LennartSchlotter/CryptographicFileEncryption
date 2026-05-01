#include "modules/decrypt.h"
#include <argon2.h>
#include <sodium/crypto_aead_aes256gcm.h>
#include <sodium/crypto_aead_chacha20poly1305.h>
#include <sodium/crypto_kdf_hkdf_sha256.h>
#include <sodium/crypto_kem.h>
#include <sodium/crypto_scalarmult.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <span>
#include <vector>

#include "handler.h"
#include "helper/cryptography.h"
#include "helper/password.h"
#include "helper/secure_allocator.h"

namespace crypto {
Result decrypt(const DecryptRequest& request) {
    const uint8_t MAGIC_BYTES_SIZE = 8;
    const uint8_t ALGORITHM_INDEX = 10;
    const char AES_256_GCM_INDEX = 0x01;
    const char ChaCha20_POLY1305_INDEX = 0x02;
    const char ECDH_X25519_INDEX = 0x10;
    const char ML_KEM_768_INDEX = 0x11;
    const std::array<char, 8> file_signature = {
        0x43, 0x46, 0x45, 0x2A, 0x5F, 0x43, 0x4C, 0x49,
    };

    // Open File
    std::ifstream source(request.request.file_path, std::ios::binary);

    if (!source.is_open()) {
        unexpected_error("Failed to open requested file.");
    }

    uint64_t file_size = std::filesystem::file_size(request.request.file_path);
    std::vector<char, SecureAllocator<char>> encrypted_file_content(file_size);
    source.read(encrypted_file_content.data(), static_cast<std::streamsize>(file_size));

    // Verify Magic Bytes
    auto byteView = std::span(encrypted_file_content).first(MAGIC_BYTES_SIZE);

    if (!std::ranges::equal(byteView, file_signature)) {
        unexpected_error("The passed file has not been encrypted using CFE and cannot be decrypted.");
    }

    // Extract Algorithm
    char algorithm_char = encrypted_file_content[ALGORITHM_INDEX];
    CryptoAlgorithms algorithm = CryptoAlgorithms::AES_256_GCM;

    switch (algorithm_char) {
        case AES_256_GCM_INDEX: {
            algorithm = CryptoAlgorithms::AES_256_GCM; 
            break;
        }
        case ChaCha20_POLY1305_INDEX: {
            algorithm = CryptoAlgorithms::ChaCha20_POLY1305; 
            break;
        }
        case ECDH_X25519_INDEX: {
            algorithm = CryptoAlgorithms::ECDH_X25519;
            break;
        }
        case ML_KEM_768_INDEX: {
            algorithm = CryptoAlgorithms::ML_KEM_768;
            break;
        }
        default: {
            unexpected_error("Header of the passed file is incorrectly formatted.");
            break;
        }
    }

    std::vector<uint8_t, SecureAllocator<uint8_t>> key;

    // Prepare
    if (is_asymmetric(algorithm)) {
        key = prepare_asymmetric(encrypted_file_content, algorithm);
    } else {
        key = prepare_symmetric(encrypted_file_content);
    }

    decrypt(encrypted_file_content, request.request.output_path, algorithm, key);

    Result result_val{.message = "Successfully decrypted the file", .success = true};
    return result_val;
}

std::vector<uint8_t, SecureAllocator<uint8_t>> prepare_symmetric(const std::vector<char, SecureAllocator<char>>& encrypted_file_content) {
    const uint8_t SALT_OFFSET = 10;
    const size_t SALT_SIZE = 16;
    const uint8_t TIME_OFFSET = 26;
    const size_t TIME_SIZE = 4;
    const uint8_t MEMORY_OFFSET = 30;
    const size_t MEMORY_SIZE = 4;
    const uint8_t PARALLELISM_OFFSET = 34;
    const size_t KEY_LENGTH = 32;

    // `read_password` with the passphrase
    std::vector<char, SecureAllocator<char>> pw = read_password("Please enter the passphrase to use for encryption: ");
    const size_t pw_len = pw.size();

    // Recompute the Password using extracted data
    std::vector<uint8_t, SecureAllocator<uint8_t>> derived_key(KEY_LENGTH);

    auto t_cost_span = std::span(encrypted_file_content.begin() + TIME_OFFSET, encrypted_file_content.begin() + TIME_OFFSET + TIME_SIZE);
    uint32_t t_cost = 0;
    std::memcpy(&t_cost, t_cost_span.data(), t_cost_span.size());

    auto m_cost_span = std::span(encrypted_file_content.begin() + MEMORY_OFFSET, encrypted_file_content.begin() + MEMORY_OFFSET + MEMORY_SIZE);
    uint32_t m_cost = 0;
    std::memcpy(&m_cost, m_cost_span.data(), m_cost_span.size());

    auto parallelism = static_cast<uint32_t>(static_cast<unsigned char>(encrypted_file_content[PARALLELISM_OFFSET]));
    auto salt = std::span(encrypted_file_content.begin() + SALT_OFFSET, encrypted_file_content.begin() + SALT_OFFSET + SALT_SIZE);

    int result = argon2id_hash_raw(t_cost, m_cost, parallelism, pw.data(), pw_len, salt.data(), salt.size(), derived_key.data(), derived_key.size());

    if (result != ARGON2_OK) {
        unexpected_error(argon2_error_message(result));
    }

    // Return the plain password
    return derived_key;
}

std::vector<uint8_t, SecureAllocator<uint8_t>> prepare_asymmetric(const std::vector<char, SecureAllocator<char>>& encrypted_file_content, const CryptoAlgorithms& algorithm) {
    const size_t KEY_LENGTH = 32;
    const size_t AES_KEY_LENGTH = 32;
    const uint8_t KEY_LENGTH_OFFSET = 10;
    const size_t KEY_LENGTH_SIZE = 4;
    const uint8_t KEY_OFFSET = 14;
    
    //`read_password` with file path to key
    std::vector<char, SecureAllocator<char>> sk_path = read_password("Please enter the path of the private key used to decrypt: ");
    using SecureString = std::basic_string<char, std::char_traits<char>, SecureAllocator<char>>;
    SecureString file_name(sk_path.begin(), sk_path.end());
    std::ifstream sk(file_name);

    std::vector<unsigned char, SecureAllocator<unsigned char>> recipient_sk((std::istreambuf_iterator<char>(sk)), std::istreambuf_iterator<char>());

    auto key_length_span = std::span(encrypted_file_content.begin() + KEY_LENGTH_OFFSET, encrypted_file_content.begin() + KEY_LENGTH_OFFSET + KEY_LENGTH_SIZE);
    uint32_t key_length = 0;
    std::memcpy(&key_length, key_length_span.data(), key_length_span.size());

    auto key_span = std::span(encrypted_file_content.begin() + KEY_OFFSET, encrypted_file_content.begin() + KEY_OFFSET + key_length);
    std::vector<unsigned char> key(key_span.begin(), key_span.end());    

    std::vector<unsigned char, SecureAllocator<unsigned char>> derived_key(KEY_LENGTH);

    switch (algorithm) {
        case CryptoAlgorithms::ECDH_X25519: {
            std::vector<unsigned char, SecureAllocator<unsigned char>> shared_point(crypto_scalarmult_BYTES);
            std::vector<unsigned char, SecureAllocator<unsigned char>> prk(crypto_kdf_hkdf_sha256_KEYBYTES);
            auto context = std::to_array("file-encryption-key");
            
            // Calculate shared point
            if (crypto_scalarmult(shared_point.data(), recipient_sk.data(), key.data()) != 0) {
                unexpected_error("Failed to calculate shared point between the passed ephemeral public key and your secret key");
            }

            // Derive key from shared point
            if (crypto_kdf_hkdf_sha256_extract(prk.data(), nullptr, 0, shared_point.data(), shared_point.size()) != 0) {
                unexpected_error("Failed to create master key");
            }

            if (crypto_kdf_hkdf_sha256_expand(derived_key.data(), AES_KEY_LENGTH, context.data(), context.size(), prk.data()) != 0) {
                unexpected_error("Failed to derive subkey from master key");
            }
            break;
        }
        case CryptoAlgorithms::ML_KEM_768: {
            std::vector<unsigned char, SecureAllocator<unsigned char>> shared_secret(crypto_kem_SHAREDSECRETBYTES);

            if (crypto_kem_dec(shared_secret.data(), key.data(), recipient_sk.data()) != 0) {
                unexpected_error("Failed to create ciphertext or secret for the passed public key");
            }
            break;
        }
        default: {
            break;
        }
    }

    return derived_key;
}

void decrypt(const std::vector<char, SecureAllocator<char>>& encrypted_file_content, const std::string& output_path, const CryptoAlgorithms& algorithm, const std::vector<unsigned char, SecureAllocator<unsigned char>>& key) {

    const uint8_t IV_OFFSET = 35;
    const size_t IV_SIZE = 12;
    const uint8_t SYMMETRIC_CIPHERTEXT_LENGTH_OFFSET = 47;
    const uint8_t SYMMETRIC_CIPHERTEXT_OFFSET = 51;
    const uint8_t ASYMMETRIC_CIPHERTEXT_LENGTH_OFFSET_BASE = 26;
    const size_t CIPHERTEXT_LENGTH_SIZE = 4;
    const size_t ASYMMETRIC_HEADER_SIZE = 50;
    const size_t SYMMETRIC_HEADER_SIZE = 51;

    unsigned long long ciphertext_len = 0; // NOLINT: API requires long long over int64
    unsigned long long decrypted_text_len = 0; // NOLINT: API requires long long over int64

    std::span<const char> header_span;
    std::vector<unsigned char, SecureAllocator<unsigned char>> header;

    // Retrieve ciphertext length
    if (is_asymmetric(algorithm)) {
        int64_t key_length = retrieve_key_length(algorithm);
        auto offset = static_cast<size_t>(ASYMMETRIC_CIPHERTEXT_LENGTH_OFFSET_BASE + key_length);

        std::span<const char> content_span(encrypted_file_content);
        std::memcpy(&ciphertext_len, content_span.data() + offset, CIPHERTEXT_LENGTH_SIZE);
    } else {
        std::span<const char> content_span(encrypted_file_content);
        std::memcpy(&ciphertext_len, content_span.data() + SYMMETRIC_CIPHERTEXT_LENGTH_OFFSET, CIPHERTEXT_LENGTH_SIZE);
    }

    std::vector<unsigned char, SecureAllocator<unsigned char>> decrypted(ciphertext_len);

    // Retreive ciphertext
    auto ciphertext_span = std::span(encrypted_file_content.begin() + SYMMETRIC_CIPHERTEXT_OFFSET, encrypted_file_content.begin() + SYMMETRIC_CIPHERTEXT_OFFSET + static_cast<int64_t>(ciphertext_len));
    std::vector<unsigned char, SecureAllocator<unsigned char>> ciphertext;
    ciphertext.assign(ciphertext_span.begin(), ciphertext_span.end());

    // Retrieve IV
    auto iv_span = std::span(encrypted_file_content.begin() + IV_OFFSET, encrypted_file_content.begin() + IV_OFFSET + IV_SIZE);
    std::vector<unsigned char> iv;
    iv.assign(iv_span.begin(), iv_span.end());

    // Retrieve header
    if (is_asymmetric(algorithm)) {
        header_span = std::span(encrypted_file_content.begin(), encrypted_file_content.begin() + ASYMMETRIC_HEADER_SIZE);
    } else {
        header_span = std::span(encrypted_file_content.begin(), encrypted_file_content.begin() + SYMMETRIC_HEADER_SIZE);
    }

    header.assign(header_span.begin(), header_span.end());

    // Decrypt that data depending on the passed algorithm using the key
    switch(algorithm) {
        case CryptoAlgorithms::ChaCha20_POLY1305: {
            if (crypto_aead_chacha20poly1305_decrypt(decrypted.data(), &decrypted_text_len, nullptr, ciphertext.data(), ciphertext.size(), header.data(), header.size(), iv.data(), key.data()) != 0) {
                unexpected_error("Failed to decrypt data. This may be due to the authentication tag being invalid.");
            }
        }
        default: {
            if (crypto_aead_aes256gcm_is_available() == 0) {
                unexpected_error("AES_256-GCM is not available on this CPU.");
            }

            if (crypto_aead_aes256gcm_decrypt(decrypted.data(), &decrypted_text_len,
                                            nullptr,
                                            ciphertext.data(), ciphertext_len,
                                            header.data(),
                                            header.size(),
                                            iv.data(), key.data()) != 0) {
                unexpected_error("Failed to decrypt data. This may be due to the authentication tag being invalid.");
            }
        }
    }

    if (output_path.ends_with(".cfe")) {
        output_path.substr(0, output_path.length() - 4);
    }
    
    std::filesystem::path path = output_path;
    std::filesystem::create_directories(path.parent_path());
    std::ofstream file(path, std::ios::out | std::ios::trunc | std::ios::binary);

    if (!file) {
        unexpected_error("Failed to open file created at output path.");
    }

    std::vector<char, SecureAllocator<char>> decrypted_char;
    std::ranges::transform(decrypted, decrypted_char.begin(), [](unsigned char character){
        return static_cast<char>(character);
    });
    file.write(decrypted_char.data(), static_cast<std::streamsize>(decrypted.size()));
}
}  // namespace crypto
