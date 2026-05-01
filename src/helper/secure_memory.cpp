#include "secure_memory.h"

#include <algorithm>

void secure_zero(void* pointer, size_t n) {
    volatile char* vp = static_cast<volatile char*>(pointer);
    std::fill_n(vp, n, 0);
}
