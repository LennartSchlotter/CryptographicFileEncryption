#include "handler.h"

#include <__expected/expected.h>
#include <__expected/unexpected.h>
#include <fmt/color.h>
#include <fmt/core.h>

#include <cstddef>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

#include "logger.h"
#include "modules/decrypt.h"
#include "modules/encrypt.h"

std::expected<void, Result> parse(std::span<const std::string_view> args) {
    const std::string tool = std::string{args[0]};
    const std::string input_path = std::string{args[1]};
    std::string output_path = input_path;
    CryptoAlgorithms algorithm = CryptoAlgorithms::ECDH;
    bool verbose = false;

    auto result = parse_flags(tool, args.subspan(2), output_path, verbose, algorithm);

    if (!result) {
        return result;
    }

    const SharedRequest shared{
        .file_path = input_path,
        .output_path = output_path,
        .verbose = verbose,
    };

    if (tool == "encrypt") {
        return dispatch(EncryptRequest{.request = shared, .algorithm = algorithm});
    }
    if (tool == "decrypt") {
        return dispatch(DecryptRequest{.request = shared});
    }

    const Result unexpected_result{.message = "Tool not found", .success = false};
    logging::log_result(unexpected_result);
    return std::unexpected(unexpected_result);
}

std::expected<void, Result> dispatch(const Request& request) {
    const Result result = std::visit(overload{
                                         [](const EncryptRequest& encrypt_request) -> Result {
                                             return crypto::encrypt(encrypt_request);
                                         },
                                         [](const DecryptRequest& decrypt_request) -> Result {
                                             return crypto::decrypt(decrypt_request);
                                         },
                                     },
                                     request);
    if (!result.success) {
        return std::unexpected(result);
    }

    return {};
}

void help() {
    fmt::print(
        "Options:\n"
        "--verbose              Start in verbose mode with enhanced logging\n"
        "-c, --cipher-algo      Specify a cipher algorithm to use. Here is a list of supported "
        "values:\n"
        "rsa, ecdh, x25519, ml_kem, aes, chacha20\n"
        "-o, --output           Specify a path for the output file to be written to\n"
        "--symmetric            Use symmetric encryption.\n"
        "--asymmetric           Use asymmetric encryption.\n"
        "-h, --help             Show this help message and exit.\n");
}

bool is_asymmetric(CryptoAlgorithms algorithm) {
    switch (algorithm) {
        case CryptoAlgorithms::RSA:
        case CryptoAlgorithms::ECDH:
        case CryptoAlgorithms::X25519:
        case CryptoAlgorithms::ML_KEM:
            return true;
            break;
        case CryptoAlgorithms::AES_256:
        case CryptoAlgorithms::ChaCha20:
            return false;
            break;
    }
}

std::expected<void, Result> parse_flags(std::string_view tool,
                                        std::span<const std::string_view> args,
                                        std::string& output_path, bool& verbose,
                                        CryptoAlgorithms& algorithm) {
    static const std::unordered_map<std::string_view, CryptoAlgorithms> algorithm_map{
        {"rsa", CryptoAlgorithms::RSA},       {"ecdh", CryptoAlgorithms::ECDH},
        {"x25519", CryptoAlgorithms::X25519}, {"ml_kem", CryptoAlgorithms::ML_KEM},
        {"aes", CryptoAlgorithms::AES_256},   {"chacha20", CryptoAlgorithms::ChaCha20},
    };

    for (auto it = args.begin(); it != args.end();) {
        auto arg = *it;

        if (arg == "-h" || arg == "--help") {
            help();
            return {};
        }

        if (arg == "--verbose") {
            verbose = true;
            ++it;
            continue;
        }

        if (arg == "-o" || arg == "--output") {
            if (++it == args.end()) {
                return unexpected_error("Output requires an argument");
            }
            output_path = static_cast<std::string>(*it);
            ++it;
            continue;
        }

        if (tool == "encrypt" && (arg == "-c" || arg == "--cipher-algo")) {
            if (++it == args.end()) {
                return unexpected_error("Cipher Algorithm requires an argument");
            }
            auto alg_str = *it;
            auto found = algorithm_map.find(alg_str);
            if (found == algorithm_map.end()) {
                return unexpected_error(
                    "Unknown cipher algorithm. Supported algorithms include: \n rsa, ecdh, x25519, "
                    "ml_kem, aes, chacha20");
            }
            algorithm = found->second;
            ++it;
            continue;
        }
        return unexpected_error("Unknown option passed: " + std::string(arg));
    }

    return {};
}

[[nodiscard]]
std::unexpected<Result> unexpected_error(std::string message) {
    Result result{.message = std::move(message), .success = false};
    logging::log_result(result);
    return std::unexpected{std::move(result)};
}