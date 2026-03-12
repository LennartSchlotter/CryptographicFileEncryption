#include "modules/encrypt.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "handler.h"
#include "logger.h"

namespace crypto {
Result encrypt(const Request& request) {
    const std::string encrypted_file_path = request.file_path + ".cfe";
    std::filesystem::copy_file(request.file_path, encrypted_file_path);
    const std::ifstream file(encrypted_file_path, std::ios::binary);
    std::cout << "You entered: " << request.file_path << "\n";
    if (!file.is_open()) {
        const Result result{.message = "Failed to open file at " + request.file_path,
                            .success = false};
        logging::log_result(result);
        return result;
    }

    Result result_val{.message = "Successfully encrypted the file", .success = true};
    return result_val;
}
}  // namespace crypto
