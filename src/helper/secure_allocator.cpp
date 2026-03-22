#include "helper/secure_allocator.h"

#include <sys/mman.h>
#include <termios.h>
#include <unistd.h>

#include <cstddef>
#include <expected>
#include <memory>
#include <string>

#include "helper/password.h"
#include "logger.h"

char* SecureAllocator::allocate(size_t n) {
    char* ptr = inner.allocate(n);
#ifdef _WIN32
    bool success = VirtualLock(ptr, n);
    if (!success) {
        logging::log("Failed to lock memory. This is a security risk.", logging::Severity::WARN);
    }
#else
    const int result = mlock(ptr, n);
    if (result == -1) {
        logging::log("Failed to lock memory. This is a security risk.", logging::Severity::WARN);
    }
#endif
    return ptr;
}

std::allocation_result<char*> SecureAllocator::allocate_at_least(size_t n) {
    std::allocation_result<char*> alloc_res = inner.allocate_at_least(n);
#ifdef _WIN32
    bool success = VirtualLock(alloc_res.ptr, alloc_res.count);
    if (!success) {
        logging::log("Failed to lock memory. This is a security risk.", logging::Severity::WARN);
    }
#else
    const int result = mlock(alloc_res.ptr, alloc_res.count);
    if (result == -1) {
        logging::log("Failed to lock memory. This is a security risk.", logging::Severity::WARN);
    }
#endif
    return alloc_res;
}

void SecureAllocator::deallocate(char* ptr, size_t n) {
    secure_zero(ptr, n);
#ifdef _WIN32
    bool success = VirtualUnlock(ptr, n);
    if (!success) {
        logging::log("Failed to unlock memory. This should not matter as it's about to be deleted.",
                     logging::Severity::WARN);
    }
#else
    const int result = munlock(ptr, n);
    if (result == -1) {
        logging::log("Failed to unlock memory. This should not matter as it's about to be deleted.",
                     logging::Severity::WARN);
    }
#endif
    inner.deallocate(ptr, n);
}
