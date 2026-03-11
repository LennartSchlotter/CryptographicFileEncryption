#ifndef CFE_HANDLER_H

#define CFE_HANDLER_H

#include <expected>
#include <optional>
#include <span>
#include <unordered_map>
#include <vector>

struct Request {
    std::string function;
    std::optional<std::unordered_map<std::string, std::vector<std::string>>> flags;
    std::string file_path;
};

struct Result {
    std::string message;
    bool success = false;
};

std::expected<void, Result> parse(std::span<const char* const> args);
std::expected<void, Result> dispatch(const Request& request);

#endif
