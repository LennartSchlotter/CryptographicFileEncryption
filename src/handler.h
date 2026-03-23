#ifndef CFE_HANDLER_H

#define CFE_HANDLER_H

#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <variant>

// The available cryptographic algorithms this tool offers to use.
enum class CryptoAlgorithms : std::uint8_t {
    RSA,
    ECDH,
    X25519,
    ML_KEM,
    AES_256,
    ChaCha20,
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
    CryptoAlgorithms algorithm = CryptoAlgorithms::AES_256;
};

// A Decryption request.
struct DecryptRequest {
    SharedRequest request;
};

// A variant over all possible kinds of requests.
using Request = std::variant<EncryptRequest, DecryptRequest>;

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

// Helper function to determine if the chosen algorithm is asymmetric
bool is_asymmetric(CryptoAlgorithms algorithm);

// Helper function to print out a generic error.
std::unexpected<Result> unexpected_error(std::string message);

#endif
