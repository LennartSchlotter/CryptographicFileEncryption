#include "logger.h"

#include <cstdlib>
#include <iostream>

#include "handler.h"

namespace logging {
void log(const Result& result) {
    if (result.success) {
        std::cout << result.message << "\n";
    }
    std::cout << "Error: " << result.message << "\n";
}
}  // namespace logging
