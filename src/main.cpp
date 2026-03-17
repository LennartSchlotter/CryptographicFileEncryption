#include <__expected/expected.h>

#include <cstdlib>
#include <span>

#include "handler.h"
#include "logger.h"

int main(int argc, char *argv[]) {
    const std::span args{argv, static_cast<std::size_t>(argc)};

    if (argc <= 2) {
        logging::log_result(Result{.message = "Too few arguments passed", .success = false});
        return EXIT_FAILURE;
    }

    std::expected<void, Result> result = parse(args);
    if (!result) {
        if (!result.error().success) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
