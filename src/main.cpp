#include <__expected/expected.h>
#include <sodium.h>
#include <sodium/core.h>

#include <cstdlib>
#include <exception>
#include <ranges>
#include <string_view>
#include <vector>

#include "handler.h"
#include "logger.h"

int main(int argc, const char* argv[]) {
    try {
        auto args_vec = std::ranges::to<std::vector<std::string_view>>(
            std::views::counted(std::next(argv), argc - 1));

        // Verify argument count
        if (argc <= 2) {
            logging::log_result(Result{.message = "Too few arguments passed.\n", .success = false});
            return EXIT_FAILURE;
        }

        // Initialize external libraries
        if (sodium_init() < 0) {
            logging::log_result(
                Result{.message = "libsodium initialization failed.\n", .success = false});
            return EXIT_FAILURE;
        }

        // Parse input
        std::expected<void, Result> result = parse(args_vec);
        if (!result) {
            if (!result.error().success) {
                return EXIT_FAILURE;
            }
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        logging::log_result(Result{.message = e.what(), .success = false});
        return EXIT_FAILURE;
    } catch (...) {
        logging::log_result(Result{.message = "Unknown exception.\n", .success = false});
        return EXIT_FAILURE;
    }
}
