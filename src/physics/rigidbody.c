#include "physics/rigidbody.h"

#include "mem.h"

void RigidbodyArray_Init(rigidbody_array_t* arr, size_t capacity){
  arr->capacity = capacity;
  arr->count = 0;

  // Ensures safety when capacity % SIMD_ALIGNMENT_BYTES != 0
  size_t vec_t_size = sizeof(vec_t) * capacity;
  vec_t_size = (vec_t_size + SIMD_ALIGNMENT_BYTES - 1) & ~(SIMD_ALIGNMENT_BYTES - 1);



  // position
  arr->px = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->py = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->pz = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);

  if (!CHECK_VALIDITY(arr->px) || !CHECK_VALIDITY(arr->py) || !CHECK_VALIDITY(arr->pz)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to position\n");
    exit(1);
  }
  
  // Velocity
  arr->vx = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->vy = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->vz = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  if (!CHECK_VALIDITY(arr->vx) || !CHECK_VALIDITY(arr->vy) || !CHECK_VALIDITY(arr->vz)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to velocity\n");
    exit(1);
  }

  // Force
  arr->fx = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->fy = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->fz = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  if (!CHECK_VALIDITY(arr->fx) || !CHECK_VALIDITY(arr->fy) || !CHECK_VALIDITY(arr->fz)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to force\n");
    exit(1);
  }
  // Torque
  arr->tx = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->ty = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->tz = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  if (!CHECK_VALIDITY(arr->tx) || !CHECK_VALIDITY(arr->ty) || !CHECK_VALIDITY(arr->tz)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to torque\n");
    exit(1);
  }
  // Angular Velocity
  arr->avx = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->avy = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->avz = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  if (!CHECK_VALIDITY(arr->avx) || !CHECK_VALIDITY(arr->avy) || !CHECK_VALIDITY(arr->avz)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to angular velocity\n");
    exit(1);
  }
  // Rotation
  arr->qx = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->qy = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->qz = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->qw = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  if (!CHECK_VALIDITY(arr->qx) || !CHECK_VALIDITY(arr->qy) || !CHECK_VALIDITY(arr->qz) || !CHECK_VALIDITY(arr->qw)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to rotation\n");
    exit(1);
  }
  // Inertia
  arr->inv_inertia_x = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->inv_inertia_y = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  arr->inv_inertia_z = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
  if (!CHECK_VALIDITY(arr->inv_inertia_x) || !CHECK_VALIDITY(arr->inv_inertia_y) || !CHECK_VALIDITY(arr->inv_inertia_z)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to inertia\n");
    exit(1);
  }
  // Properties
  arr->mass = aligned_alloc(SIMD_ALIGNMENT_BYTES, vec_t_size);
}

void RigidbodyArray_Destroy(rigidbody_array_t* arr){
  free(arr->px); free(arr->py); free(arr->pz);
  arr->px = NULL; arr->py = NULL; arr->pz = NULL;

  free(arr->vx); free(arr->vy); free(arr->vz);
  arr->vx = NULL; arr->vy = NULL; arr->vz = NULL;

  free(arr->fx); free(arr->fy); free(arr->fz);
  arr->fx = NULL; arr->fy = NULL; arr->fz = NULL;

  free(arr->tx); free(arr->ty); free(arr->tz);
  arr->tx = NULL; arr->ty = NULL; arr->tz = NULL;

  free(arr->avx); free(arr->avy); free(arr->avz);
  arr->avx = NULL; arr->avy = NULL; arr->avz = NULL;

  free(arr->qx); free(arr->qy); free(arr->qz); free(arr->qw);
  arr->qx = NULL; arr->qy = NULL; arr->qz = NULL; arr->qw = NULL;

  free(arr->inv_inertia_x); free(arr->inv_inertia_y); free(arr->inv_inertia_z);
  arr->inv_inertia_x = NULL; arr->inv_inertia_y = NULL; arr->inv_inertia_z = NULL;

  free(arr->mass);
  arr->mass = NULL;
}
