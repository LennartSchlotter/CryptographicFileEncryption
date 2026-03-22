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

int main(int argc, char *argv[]) {
    try {
        auto args_vec = std::ranges::to<std::vector<std::string_view>>(
            std::views::counted(std::next(argv), argc - 1)
        );

        if (argc <= 2) {
            logging::log_result(Result{.message = "Too few arguments passed", .success = false});
            return EXIT_FAILURE;
        }

        if (sodium_init() < 0) {
            logging::log_result(Result{.message = "libsodium initialization failed.", .success = false});
            return EXIT_FAILURE;
        }

        std::expected<void, Result> result = parse(args_vec);
        if (!result) {
            if (!result.error().success) {
                return EXIT_FAILURE;
            }
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        logging::log_result(Result {.message = e.what(), .success = false});
        return EXIT_FAILURE;
    } catch (...) {
        logging::log_result(Result {.message = "Unknown exception", .success = false});
        return EXIT_FAILURE;
    }
}
