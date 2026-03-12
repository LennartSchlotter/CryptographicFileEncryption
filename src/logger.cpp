#include "logger.h"

#include <cstdlib>
#include <iostream>

#include "handler.h"

namespace logging {
void log_result(const Result& result) {
    if (result.success) {
        std::cout << result.message << "\n";
    }
    std::cout << "Error: " << result.message << "\n";
}

void log(const std::string& message) {
    std::cout << "[LOG] " << message;
}

}  // namespace logging
