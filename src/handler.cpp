#include "handler.h"

#include <cstddef>
#include <expected>
#include <functional>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#include "logger.h"
#include "modules/decrypt.h"
#include "modules/encrypt.h"

std::expected<void, Result> parse(std::span<const char* const> args) {
    std::string tool;
    std::string input_path;
    std::unordered_map<std::string, std::vector<std::string>> flags;

    tool = args[1];
    input_path = args[2];

    const size_t arg_size = args.size();

    size_t index = 3;
    if (index < args.size()) {
        std::string flag;
        flag = args[index];
        ++index;

        if (!flag.starts_with('-')) {
            const Result result{.message = "Expected flag, got positional: " + flag,
                                .success = false};
            logging::log_result(result);
            return std::unexpected(result);
        }

        std::string arg;
        flags.insert({flag, std::vector<std::string>()});
        while (index < arg_size) {
            arg = args[index];
            if (arg.starts_with('-')) {
                flags.insert({arg, std::vector<std::string>()});
                flag = arg;
            } else {
                auto vec = flags.find(flag);
                if (vec != flags.end()) {
                    vec->second.emplace_back(arg);
                }
            }
            ++index;
        }
    }

    const Request request{.function = tool, .flags = flags, .file_path = input_path};
    return dispatch(request);
}

std::expected<void, Result> dispatch(const Request& request) {
    using Handler = std::function<Result(const Request&)>;
    static const std::unordered_map<std::string, Handler> dispatch_table{
        {"encrypt", [](const Request& request) -> Result { return crypto::encrypt(request); }},
        {"decrypt", [](const Request& request) -> Result { return crypto::decrypt(request); }},
    };

    auto func = dispatch_table.find(request.function);

    if (func == dispatch_table.end()) {
        const Result result{.message = "Tool not found", .success = false};
        logging::log_result(result);
        return std::unexpected(result);
    }
    Result result = func->second(request);
    return {};
}
