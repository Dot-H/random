#pragma onc

#include <new>
#include <stdlib.h>

#define UNLIKELY(x) (__builtin_expect((x), 0))

/**
 * Trivial wrappers around malloc, calloc, realloc that check for allocation
 * failure and throw std::bad_alloc in that case.
 */
inline void* checkedMalloc(size_t size) {
  void* p = malloc(size);
  if (!p) {
    throw std::bad_alloc();
  }

  return p;
}
