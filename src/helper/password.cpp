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

#ifdef _WIN32
struct TerminalGuard {
   private:
    HANDLE hStdin = INVALID_HANDLE_VALUE;
    DWORD old_mode{};
    bool valid = false;

   public:
    TerminalGuard() : hStdin(GetStdHandle(STD_INPUT_HANDLE)) {
        if (hStdin == INVALID_HANDLE_VALUE) {
            logging::log("Failed to retrieve standard input handle. Echoing cannot be disabled",
                         logging::Severity::WARN);
            return;
        }

        if (!GetConsoleMode(hStdin, &old_mode)) {
            logging::log("Failed to retrieve console mode. Echoing cannot be disabled",
                         logging::Severity::WARN);
            return;
        }

        if (!SetConsoleMode(hStdin, old_mode & (~ENABLE_ECHO_INPUT))) {
            logging::log("Failed to set console mode. Echoing cannot be disabled",
                         logging::Severity::WARN);
            return;
        }
        valid = true;
    }

    ~TerminalGuard() {
        if (valid) {
            if (!SetConsoleMode(hStdin, old_mode)) {
                logging::log("Failed to set console mode. Echoing cannot be re-enabled",
                             logging::Severity::WARN);
            }
        }
    }
    TerminalGuard(const TerminalGuard&) = delete;
    TerminalGuard& operator=(const TerminalGuard&) = delete;
    TerminalGuard(TerminalGuard&&) = delete;
    TerminalGuard& operator=(TerminalGuard&&) = delete;
};
#else
struct TerminalGuard {
   private:
    termios old_t{};
    bool valid = false;

   public:
    TerminalGuard() {
        if (tcgetattr(STDIN_FILENO, &old_t) == -1) {
            logging::log("Failed to get console mode. Echoing cannot be disabled",
                         logging::Severity::WARN);
            return;
        }
        termios new_t = old_t;
        new_t.c_lflag &= ~static_cast<tcflag_t>(ECHO);
        if (tcsetattr(STDIN_FILENO, TCSANOW, &new_t) == -1) {
            logging::log("Failed to set console mode. Echoing cannot be disabled",
                         logging::Severity::WARN);
            return;
        }
        valid = true;
    }

    ~TerminalGuard() {
        if (valid) {
            if (tcsetattr(STDIN_FILENO, TCSANOW, &old_t) == -1) {
                logging::log("Failed to set console mode. Echoing cannot be disabled",
                             logging::Severity::WARN);
            }
        }
    }

    TerminalGuard(const TerminalGuard&) = delete;
    TerminalGuard& operator=(const TerminalGuard&) = delete;
    TerminalGuard(TerminalGuard&&) = delete;
    TerminalGuard& operator=(TerminalGuard&&) = delete;
};
#endif

std::vector<char, SecureAllocator<char>> read_password(const std::string& message) {
    static constexpr size_t kInitialPasswordCapacity = 128;
    std::cout << message << std::flush;

    const TerminalGuard guard;

    std::vector<char, SecureAllocator<char>> password;
    password.reserve(kInitialPasswordCapacity);
    char character = 0;

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

    std::cout << "\n";
    return password;
}
