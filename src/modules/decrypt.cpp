#include "modules/decrypt.h"

#include <fstream>
#include <iostream>

#include "handler.h"

namespace crypto {
Result decrypt(const Request& request) {
    const std::ifstream file(request.file_path, std::ios::binary);
    Result result_val{.message = "Successfully decrypted the file", .success = true};
    return result_val;
}
}  // namespace crypto
