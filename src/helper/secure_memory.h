#ifndef SECURE_MEMORY_H

#define SECURE_MEMORY_H

#include <cstddef>

// Provides a secure way to overwrite a memory region.
void secure_zero(void* pointer, size_t n);

#endif
