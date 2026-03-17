#include "password.h"

#include <sys/mman.h>
#include <termios.h>
#include <unistd.h>

#include <cstddef>
#include <expected>
#include <iostream>
#include <ostream>
#include <string>

#include "logger.h"
#include "secure_allocator.h"

#ifdef _WIN32
#include <windows.h>
#endif

std::vector<char, SecureAllocator> read_password(const std::string& message) {
    static constexpr size_t kInitialPasswordCapacity = 128;

    std::cout << message << std::flush;

    std::vector<char, SecureAllocator> password;
    password.reserve(kInitialPasswordCapacity);

    char character = 0;

#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE) {
        logging::log("Failed to retrieve standard input handle. Echoing cannot be disabled",
                     logging::Severity::WARN);
    }

    DWORD mode = 0;
    bool success = GetConsoleMode(hStdin, &mode);
    if (!success) {
        logging::log("Failed to retrieve console mode. Echoing cannot be disabled",
                     logging::Severity::WARN);
    }
    success = SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
    if (!success) {
        logging::log("Failed to set console mode. Echoing cannot be disabled",
                     logging::Severity::WARN);
    }

    while (std::cin.get(character)) {
        if (character == '\r') {
            std::cin.ignore();
            break;
        }
        if (character == '\n') {
            break;
        }
        password.push_back(character);
    }

    success = SetConsoleMode(hStdin, mode);
    if (!success) {
        logging::log("Failed to set console mode. Echoing cannot be disabled",
                     logging::Severity::WARN);
    }
#else
    termios old_t{};
    int result = tcgetattr(STDIN_FILENO, &old_t);
    if (result == -1) {
        logging::log("Failed to get console mode. Echoing cannot be disabled",
                     logging::Severity::WARN);
    }

    termios new_t = old_t;
    new_t.c_lflag &= ~static_cast<tcflag_t>(ECHO);
    result = tcsetattr(STDIN_FILENO, TCSANOW, &new_t);
    if (result == -1) {
        logging::log("Failed to set console mode. Echoing cannot be disabled",
                     logging::Severity::WARN);
    }

    while (std::cin.get(character)) {
        if (character == '\r') {
            std::cin.ignore();
            break;
        }
        if (character == '\n') {
            break;
        }
        password.push_back(character);
    }

    result = tcsetattr(STDIN_FILENO, TCSANOW, &old_t);
    if (result == -1) {
        logging::log("Failed to set console mode. Echoing cannot be disabled",
                     logging::Severity::WARN);
    }
#endif

    std::cout << "\n";
    return password;
}

void secure_zero(void* pointer, size_t n) {
    volatile char* vp = static_cast<volatile char*>(pointer);
    std::fill_n(vp, n, 0);
}
