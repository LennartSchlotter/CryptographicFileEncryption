#ifndef CFE_LOGGER_H

#define CFE_LOGGER_H

#include "handler.h"

namespace logging {

enum Severity : std::uint8_t {
    DEBUG,
    WARN,
    ERROR,
};

// Logs the result back to the user.
void log_result(const Result& result);

// Writes any message to the output.
void log(const std::string& message, Severity severity);
}  // namespace logging

#endif
