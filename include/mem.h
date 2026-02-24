#ifndef MEM_H
#define MEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdalign.h>

#define SIMD_ALIGNMENT_BYTES 32

#if defined(_MSC_VER)
#include <malloc.h>
#define ALIGNED_ALLOC(alignment, size) _aligned_malloc(size, alignment)
#define ALIGNED_FREE(ptr) _aligned_free(ptr)
#define ALIGNED_REALLOC(ptr, new_size) _aligned_realloc(ptr, new_size, SIMD_ALIGNMENT_BYTES)
#else
#include <stdlib.h>

// Simple aligned allocation
#define ALIGNED_ALLOC(alignment, size) aligned_alloc(alignment, size)
#define ALIGNED_FREE(ptr) free(ptr)

// Aligned realloc wrapper storing old size
typedef struct {
    void* ptr;
    size_t size; // size in bytes
} aligned_block_t;

// Allocate a new aligned block
static inline aligned_block_t aligned_alloc_block(size_t size, size_t alignment) {
    size_t padded_size = ((size + alignment - 1) / alignment) * alignment;
    aligned_block_t blk;
    blk.ptr = aligned_alloc(alignment, padded_size);
    blk.size = blk.ptr ? size : 0;
    return blk;
}

// Reallocate an aligned block
static inline aligned_block_t aligned_realloc_block(aligned_block_t blk, size_t new_size, size_t alignment) {
    size_t padded_size = ((new_size + alignment - 1) / alignment) * alignment;
    void* new_ptr = aligned_alloc(alignment, padded_size);
    if (new_ptr) {
        if (blk.ptr) {
            size_t copy_size = blk.size < new_size ? blk.size : new_size;
            memcpy(new_ptr, blk.ptr, copy_size);
            free(blk.ptr);
        }
        blk.ptr = new_ptr;
        blk.size = new_size;
    } else {
        blk.ptr = NULL;
        blk.size = 0;
    }
    return blk;
}

// Macros for convenience
#define ALIGNED_REALLOC(blk, new_size) aligned_realloc_block(blk, new_size, SIMD_ALIGNMENT_BYTES)
#define ALIGNED_NEW(size) aligned_alloc_block(size, SIMD_ALIGNMENT_BYTES)

#endif

int mem_validity(void* mem);

#define CHECK_VALIDITY(x) (mem_validity(x))


typedef void* MEM_BLOCK;
#define MEM_ARENA_SIZE_BYTES (size_t)2048 // 2kiB

struct mem_arena_t{
  uint8_t* base;
  size_t capacity;
  size_t offset;
};

void MEM_ARENA_INIT(struct mem_arena_t* arena);
void MEM_ARENA_RESET(struct mem_arena_t* arena);
void MEM_ARENA_DESTROY(struct mem_arena_t* arena);
void* MEM_ARENA_ALLOC(struct mem_arena_t* arena, size_t size, size_t alignment);

#endif
