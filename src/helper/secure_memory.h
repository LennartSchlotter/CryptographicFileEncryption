#ifndef SECURE_MEMORY_H

#define SECURE_MEMORY_H

#include <cstddef>

// Provides a secure way to overwrite a memory region.
// This function is to be used with caution, as it gives the user full control of the executed
// behavior. As such, undefined behavior like buffer overflow or memory corruption is possible. It
// is advised to validate the pointer called on this function to avoid undefined behavior.
void secure_zero(void* pointer, size_t n);

#endif
