#ifndef PASSWORD_H

#define PASSWORD_H

#include <string>
#include <vector>

#include "helper/secure_allocator.h"

struct TerminalGuard;

// Securely reads a string from the command line.
std::vector<char, SecureAllocator<char>> read_secure(const std::string& message, bool disable_echo);

#endif
