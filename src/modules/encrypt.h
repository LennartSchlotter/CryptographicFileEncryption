#ifndef ENCRYPT_H

#define ENCRYPT_H

#include "handler.h"

namespace crypto {

// Handles encrypting the file passed in the request.
Result encrypt(const EncryptRequest& request);
}  // namespace crypto

#endif
