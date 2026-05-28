#ifndef PASSWORD_H

#define PASSWORD_H

#include <string>
#include <vector>

#include "helper/secure_allocator.h"

struct TerminalGuard;

// Reads a key from the commandline.
std::vector<char, SecureAllocator<char>> read_password(const std::string& message, bool disable_echo);

#endif
