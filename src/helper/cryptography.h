#ifndef CRYPTOGRAPHY_H

#define CRYPTOGRAPHY_H

#include "handler.h"

// Helper function to determine if the chosen algorithm is asymmetric
bool is_asymmetric(CryptoAlgorithms algorithm);

// Helper function to determine the passed key length for asymmetric algorithms
int64_t retrieve_key_length(CryptoAlgorithms algorithm);

#endif
