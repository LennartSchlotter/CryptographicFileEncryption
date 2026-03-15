#ifndef PASSWORD_H

#define PASSWORD_H

#include <string>
#include <vector>

// Reads a key from the commandline.
// This function uses the preprocessor for conditional compilation.
// There is an argument to be made to use separate compilation entries and have CMake determine
// which to use
std::vector<char> read_password(const std::string& message);

#endif
