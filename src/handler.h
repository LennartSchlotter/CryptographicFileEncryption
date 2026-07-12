#ifndef CFE_HANDLER_H

#define CFE_HANDLER_H

#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <variant>

// The available cryptographic algorithms this tool offers to use.
enum class CryptoAlgorithms : std::uint8_t {
    AEGIS_256 = 0x01,
    ChaCha20_POLY1305 = 0x02,
    ECDH_X25519 = 0x10,
    ML_KEM_768 = 0x11,
};

// Contains the shared fields that any request has
struct SharedRequest {
    std::string file_path;
    std::string output_path;
    bool verbose;
};

// An Encryption request.
struct EncryptRequest {
    SharedRequest request;
    CryptoAlgorithms algorithm = CryptoAlgorithms::AEGIS_256;
};

// A Decryption request.
struct DecryptRequest {
    SharedRequest request;
};

// A Key generation request.
struct KeygenRequest {
    SharedRequest request;
    CryptoAlgorithms algorithm = CryptoAlgorithms::ECDH_X25519;
};

// A variant over all possible kinds of requests.
using Request = std::variant<EncryptRequest, DecryptRequest, KeygenRequest>;

// Represents a result of the request.
struct Result {
    std::string message;
    bool success = false;
};

template <class... Ts>
struct overload : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overload(Ts...) -> overload<Ts...>;

// Parses the args passed to the application into a request object.
std::expected<void, Result> parse(std::span<const std::string_view> args);

// Handles individual parsing of the flags passed to a request.
std::expected<void, Result> parse_flags(std::string_view tool,
                                        std::span<const std::string_view> args,
                                        std::string& output_path, bool& verbose,
                                        CryptoAlgorithms& algorithm);

// Dispatches the module responsible for handling the request.
std::expected<void, Result> dispatch(const Request& request);

// Prints a help message
void help();

// Helper function to print out a generic error.
std::unexpected<Result> unexpected_error(std::string message);

#endif
