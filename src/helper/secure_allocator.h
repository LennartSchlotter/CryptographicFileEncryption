#ifndef SECURE_ALLOCATOR_H

#define SECURE_ALLOCATOR_H

#include <sys/mman.h>
#include <termios.h>
#include <unistd.h>

#include <cstddef>
#include <expected>
#include <memory>

#include "helper/password.h"

// A custom wrapper for std::allocator, inserting the provided secure_zero before deallocating.
class SecureAllocator {
   private:
    std::allocator<char> inner;

   public:
    using value_type = char;
    template <typename U>
    struct rebind {
        using other = SecureAllocator;
    };

    // Allocates memory for the passed amount of chars.
    char* allocate(size_t n);

    // Allocates at least the passed amount of chars.
    // Returns an allocation result containing a pointer to the allocated memory and the actual number of elements.
    std::allocation_result<char*> allocate_at_least(size_t n);

    // Provides a way to deallocate memory using the built-in `secure_zero` helper.
    void deallocate(char* ptr, size_t n);
};

#endif
