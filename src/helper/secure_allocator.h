#ifndef SECURE_ALLOCATOR_H

#define SECURE_ALLOCATOR_H

#include <cstddef>
#include <memory>

// Generic template for the type to be used for the allocator.
template <typename T>

// A custom wrapper for std::allocator, inserting the provided secure_zero before deallocating.
class SecureAllocator {
   private:
    std::allocator<T> inner;

   public:
    using value_type = T;

    // Allocates memory for the passed amount of objects.
    T* allocate(size_t n);

    // Allocates at least the passed amount of objects.
    // Returns an allocation result containing a pointer to the allocated memory and the actual number of elements.
    std::allocation_result<T*> allocate_at_least(size_t n);

    // Provides a way to deallocate memory using the built-in `secure_zero` helper.
    void deallocate(T* ptr, size_t n);

    bool operator==(const SecureAllocator&  /*other*/) const noexcept {
        return true;
    }

    bool operator!=(const SecureAllocator&  other) const noexcept {
        return !(*this == other);
    }
};

#endif
