
#ifndef DECRYPT_H

#define DECRYPT_H

#include "handler.h"

namespace crypto {

// Handles decrypting the file passed in the request.
Result decrypt(const Request& request);
}  // namespace crypto

#endif
