#include "logger.h"

#include <cstdlib>
#include <iostream>

#include "handler.h"

namespace logging {
void log_result(const Result& result) {
    if (result.success) {
        std::cout << result.message << "\n";
    } else {
        std::cout << "Error: " << result.message << "\n";
    }
}

void log(const std::string& message, Severity severity = DEBUG) {
    switch (severity) {
        default: std::cout << message << "\n";
    }
}

}  // namespace logging
