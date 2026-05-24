#ifndef MEM_H
#define MEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdalign.h>

#define SIMD_ALIGNMENT_BYTES 32

// -------------------------
// Bit flags
// -------------------------
#define BIT64(n) ((size_t)1ULL << (n))

#define SET_FLAG_BIT(mask, n)   ((mask) |= BIT64(n)) // bit index
#define CLR_FLAG_BIT(mask, n)   ((mask) &= ~BIT64(n)) // bit index
#define HAS_FLAG_BIT(mask, n)   ((mask) & BIT64(n)) // bit index
#define TOG_FLAG_BIT(mask, n)   ((mask) ^= BIT64(n)) // bit index

// -------------------------
// Aligned allocation (cross-platform unified API)
// -------------------------

#if defined(_WIN32)

#include <malloc.h>

// Windows aligned allocation
#define ALIGNED_NEW(size) \
    _aligned_malloc((size), SIMD_ALIGNMENT_BYTES)

#define ALIGNED_REALLOC(ptr, old_size, new_size) \
    _aligned_realloc((ptr), (new_size), SIMD_ALIGNMENT_BYTES)

#define ALIGNED_FREE(ptr) \
    _aligned_free((ptr))

#else

// -------------------------
// POSIX / MSYS2 / Linux fallback
// -------------------------

static inline void* aligned_new(size_t size) {
    void* ptr = NULL;
    if (posix_memalign(&ptr, SIMD_ALIGNMENT_BYTES, size) != 0) {
        return NULL;
    }
    return ptr;
}

static inline void* aligned_realloc(void* ptr, size_t old_size, size_t new_size) {
    void* new_ptr = aligned_new(new_size);
    if (!new_ptr) return NULL;

    if (ptr && old_size > 0) {
        size_t copy_size = old_size < new_size ? old_size : new_size;
        memcpy(new_ptr, ptr, copy_size);
        free(ptr);
    }

    return new_ptr;
}

#define ALIGNED_NEW(size) \
    aligned_new((size))

#define ALIGNED_REALLOC(ptr, old_size, new_size) \
    aligned_realloc((ptr), (old_size), (new_size))

#define ALIGNED_FREE(ptr) \
    free((ptr))

#endif

// -------------------------
// Memory validity helper
// -------------------------
int mem_validity(void* mem);

#define CHECK_VALIDITY(x) (mem_validity(x))

// -------------------------
// Arena allocator
// -------------------------
typedef void* MEM_BLOCK;

#define MEM_ARENA_SIZE_BYTES ((size_t)8192)

typedef struct mem_arena_t {
    uint8_t* base;
    size_t capacity;
    size_t offset;
} mem_arena_t;

#ifdef __cplusplus
extern "C" {
#endif

extern mem_arena_t* gMemArena;

void MEM_ARENA_INIT(mem_arena_t* arena);
void MEM_ARENA_RESET(mem_arena_t* arena);
void MEM_ARENA_DESTROY(mem_arena_t* arena);

void* MEM_ARENA_ALLOC(mem_arena_t* arena, size_t size, size_t alignment);

#ifdef __cplusplus
}
#endif

#endif // MEM_H