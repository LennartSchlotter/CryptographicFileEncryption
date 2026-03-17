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

    char* allocate(size_t n);
    std::allocation_result<char*> allocate_at_least(size_t n);
    void deallocate(char* ptr, size_t n);
};

#endif
