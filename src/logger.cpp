#include "logger.h"

#include <fmt/color.h>

#include <cstdlib>
#include <iostream>

#include "handler.h"

namespace logging {
void log_result(const Result& result) {
    if (result.success) {
        std::cout << result.message << "\n";
    } else {
        fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, "[ERROR] {}\n", result.message);
    }
}

void log(const std::string& message, Severity severity) {
    switch (severity) {
        case Severity::ERROR:
            fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, "{}\n", message);
            break;
        case Severity::WARN:
            fmt::print(fg(fmt::color::yellow), "{}\n", message);
            break;
        case Severity::DEBUG:
            fmt::print("[DEBUG] {}\n", message);
            break;
        default:
            std::cout << message << "\n";
            break;
    }
}

}  // namespace logging
