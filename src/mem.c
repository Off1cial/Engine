#include "mem.h"


int mem_validity(void* mem){
  if (NULL == mem){
    fprintf(stderr, "Invalid Memory\n");
    return 0;
  }
  return 1;
}


void MEM_ARENA_INIT(struct mem_arena_t* arena){
  if (!arena){
    fprintf(stderr, "[MEM_ARENA]: Null arena passed to initialiser\n");
    exit(1);
  }
  arena->base = malloc(MEM_ARENA_SIZE_BYTES);
  if (!arena->base){
    fprintf(stderr, "[MEM_ARENA]: Arena base returned null\n");
    exit(1);
  }
  arena->capacity = MEM_ARENA_SIZE_BYTES;
  arena->offset = 0;

  printf("[MEM_ARENA]: Arena of created of size: %zuB\n", MEM_ARENA_SIZE_BYTES);
}

void MEM_ARENA_RESET(struct mem_arena_t* arena){
  arena->offset = 0;
}

void MEM_ARENA_DESTROY(struct mem_arena_t* arena){
  free(arena->base);
  arena->base = NULL;
  arena->offset = 0;
  arena->capacity = 0;

}

MEM_BLOCK MEM_ARENA_ALLOC(struct mem_arena_t* arena, size_t size, size_t alignment){
  uintptr_t base = (uintptr_t)arena->base;  // cast base pointer to integer
  uintptr_t cAddr = (uintptr_t)(arena->offset + base);
  // Round to alignment
  uintptr_t  alAddr = (cAddr + (alignment - 1)) & ~(alignment -1);

  size_t padding = alAddr - cAddr;
  size_t new_offset = arena->offset + padding + size;
  if (new_offset > arena->capacity){
    fprintf(stderr, "[MEM_ARENA]: Limit reached, consider implementing growth\n");
    exit(1);
  }
  arena->offset = new_offset;
  return (MEM_BLOCK)(alAddr);
}
