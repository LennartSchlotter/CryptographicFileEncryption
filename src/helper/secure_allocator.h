#ifndef SECURE_ALLOCATOR_H

#define SECURE_ALLOCATOR_H

#include <sys/mman.h>
#include <termios.h>
#include <unistd.h>

#include <cstddef>
#include <expected>
#include <memory>
#include <string>

#include "logger.h"
#include "secure_memory.h"

// Generic template for the type to be used for the allocator.
template <typename T>

// A custom wrapper for std::allocator, inserting the provided secure_zero before deallocating.
class SecureAllocator {
   private:
    std::allocator<T> inner;

   public:
    using value_type = T;

    // Allocates memory for the passed amount of objects.
    T* allocate(size_t n) {
        char* ptr = inner.allocate(n);
#ifdef _WIN32
        bool success = VirtualLock(ptr, n * sizeof(T));
        if (!success) {
            logging::log("Failed to lock memory. This is a security risk.\n",
                         logging::Severity::WARN);
        }
#else
        const int result = mlock(ptr, n * sizeof(T));
        if (result == -1) {
            logging::log("Failed to lock memory. This is a security risk.\n",
                         logging::Severity::WARN);
        }
#endif
        return ptr;
    }

    // Allocates at least the passed amount of objects.
    // Returns an allocation result containing a pointer to the allocated memory and the actual
    // number of elements.
    std::allocation_result<T*> allocate_at_least(size_t n) {
        std::allocation_result<T*> alloc_res = inner.allocate_at_least(n);
#ifdef _WIN32
        bool success = VirtualLock(alloc_res.ptr, alloc_res.count);
        if (!success) {
            logging::log("Failed to lock memory. This is a security risk.\n",
                         logging::Severity::WARN);
        }
#else
        const int result = mlock(alloc_res.ptr, alloc_res.count);
        if (result == -1) {
            logging::log("Failed to lock memory. This is a security risk.\n",
                         logging::Severity::WARN);
        }
#endif
        return alloc_res;
    }

    // Provides a way to deallocate memory using the built-in `secure_zero` helper.
    void deallocate(T* ptr, size_t n) {
        secure_zero(ptr, n);
#ifdef _WIN32
        bool success = VirtualUnlock(ptr, n * sizeof(T));
        if (!success) {
            logging::log(
                "Failed to unlock memory. This should not matter as it's about to be deleted.\n",
                logging::Severity::WARN);
        }
#else
        const int result = munlock(ptr, n * sizeof(T));
        if (result == -1) {
            logging::log(
                "Failed to unlock memory. This should not matter as it's about to be deleted.\n",
                logging::Severity::WARN);
        }
#endif
        inner.deallocate(ptr, n);
    }

    // Defines the equality operator.
    bool operator==(const SecureAllocator& /*other*/) const noexcept {
        return true;
    }

    // Defines the inequality operator.
    bool operator!=(const SecureAllocator& other) const noexcept {
        return !(*this == other);
    }
};

#endif
