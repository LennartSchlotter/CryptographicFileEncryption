#ifndef PASSWORD_H

#define PASSWORD_H

#include <string>
#include <vector>

class SecureAllocator;

struct TerminalGuard;

// Reads a key from the commandline.
// This function uses the preprocessor for conditional compilation.
// There is an argument to be made to use separate compilation entries and have CMake determine
// which to use
std::vector<char, SecureAllocator> read_password(const std::string& message);

// Provides a secure way to overwrite a memory region.
void secure_zero(void* pointer, size_t n);

#endif
