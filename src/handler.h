#ifndef CFE_HANDLER_H

#define CFE_HANDLER_H

#include <expected>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

// Represents an invocation of the tool, including the used functionality, the file path and any
// optional flags passed
struct Request {
    std::string function;
    std::optional<std::unordered_map<std::string, std::vector<std::string>>> flags;
    std::string file_path;
};

// Represents a result of the request.
struct Result {
    std::string message;
    bool success = false;
};

// Parses the args passed to the application into a request object.
std::expected<void, Result> parse(std::span<const char* const> args);

// Dispatches the module responsible for handling the request.
std::expected<void, Result> dispatch(const Request& request);

#endif
