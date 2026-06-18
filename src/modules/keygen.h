
#ifndef KEYGEN_H

#define KEYGEN_H

#include "handler.h"

namespace crypto {

// Handles generating a keypair for the passed algorithm.
Result keygen(const KeygenRequest& request);
}  // namespace crypto

#endif
