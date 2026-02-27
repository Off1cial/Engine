#include "physics/rigidbody.h"

#include "mem.h"

void RigidbodyArray_Init(rigidbody_array_t* arr, size_t capacity){
  arr->capacity = capacity;
  arr->count = 0;

  // Ensures safety when capacity % SIMD_ALIGNMENT_BYTES != 0
  size_t vec_t_size = sizeof(vec_t) * capacity;
  vec_t_size = (vec_t_size + SIMD_ALIGNMENT_BYTES - 1) & ~(SIMD_ALIGNMENT_BYTES - 1);



  // position
  arr->px = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->py = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->pz = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);

  if (!CHECK_VALIDITY(arr->px) || !CHECK_VALIDITY(arr->py) || !CHECK_VALIDITY(arr->pz)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to position\n");
    exit(1);
  }
  
  // Velocity
  arr->vx = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->vy = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->vz = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  if (!CHECK_VALIDITY(arr->vx) || !CHECK_VALIDITY(arr->vy) || !CHECK_VALIDITY(arr->vz)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to velocity\n");
    exit(1);
  }

  // Force
  arr->fx = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->fy = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->fz = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  if (!CHECK_VALIDITY(arr->fx) || !CHECK_VALIDITY(arr->fy) || !CHECK_VALIDITY(arr->fz)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to force\n");
    exit(1);
  }
  // Torque
  arr->tx = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->ty = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->tz = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  if (!CHECK_VALIDITY(arr->tx) || !CHECK_VALIDITY(arr->ty) || !CHECK_VALIDITY(arr->tz)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to torque\n");
    exit(1);
  }
  // Angular Velocity
  arr->avx = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->avy = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->avz = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  if (!CHECK_VALIDITY(arr->avx) || !CHECK_VALIDITY(arr->avy) || !CHECK_VALIDITY(arr->avz)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to angular velocity\n");
    exit(1);
  }
  // Rotation
  arr->qx = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->qy = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->qz = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->qw = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  if (!CHECK_VALIDITY(arr->qx) || !CHECK_VALIDITY(arr->qy) || !CHECK_VALIDITY(arr->qz) || !CHECK_VALIDITY(arr->qw)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to rotation\n");
    exit(1);
  }
  // Inertia
  arr->inv_inertia_x = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->inv_inertia_y = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->inv_inertia_z = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
  if (!CHECK_VALIDITY(arr->inv_inertia_x) || !CHECK_VALIDITY(arr->inv_inertia_y) || !CHECK_VALIDITY(arr->inv_inertia_z)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to inertia\n");
    exit(1);
  }
  // Properties
  arr->mass = ALIGNED_ALLOC(SIMD_ALIGNMENT_BYTES, vec_t_size);
}


void RigidbodyArray_Destroy(rigidbody_array_t* arr){
  ALIGNED_FREE(arr->px); ALIGNED_FREE(arr->py); ALIGNED_FREE(arr->pz);
  arr->px = NULL; arr->py = NULL; arr->pz = NULL;

  ALIGNED_FREE(arr->vx); ALIGNED_FREE(arr->vy); ALIGNED_FREE(arr->vz);
  arr->vx = NULL; arr->vy = NULL; arr->vz = NULL;

  ALIGNED_FREE(arr->fx); ALIGNED_FREE(arr->fy); ALIGNED_FREE(arr->fz);
  arr->fx = NULL; arr->fy = NULL; arr->fz = NULL;

  ALIGNED_FREE(arr->tx); ALIGNED_FREE(arr->ty); ALIGNED_FREE(arr->tz);
  arr->tx = NULL; arr->ty = NULL; arr->tz = NULL;

  ALIGNED_FREE(arr->avx); ALIGNED_FREE(arr->avy); ALIGNED_FREE(arr->avz);
  arr->avx = NULL; arr->avy = NULL; arr->avz = NULL;

  ALIGNED_FREE(arr->qx); ALIGNED_FREE(arr->qy); ALIGNED_FREE(arr->qz); ALIGNED_FREE(arr->qw);
  arr->qx = NULL; arr->qy = NULL; arr->qz = NULL; arr->qw = NULL;

  ALIGNED_FREE(arr->inv_inertia_x); ALIGNED_FREE(arr->inv_inertia_y); ALIGNED_FREE(arr->inv_inertia_z);
  arr->inv_inertia_x = NULL; arr->inv_inertia_y = NULL; arr->inv_inertia_z = NULL;

  ALIGNED_FREE(arr->mass);
  arr->mass = NULL;
}
