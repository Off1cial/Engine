#include "physics/rigidbody.h"
#include "physics/collider.h"

#include "mem.h"


void compute_box_inertia(float rho, Vector size, mat3 inertia)
{
  float w = size.x;
  float h = size.y;
  float d = size.z;
  
  float m = w * h * d * rho;

  inertia.m[0][0] = (1.0f / 12.0f) * m * (h * h + d * d);
  inertia.m[1][1] = (1.0f / 12.0f) * m * (w * w + d * d);
  inertia.m[2][2] = (1.0f / 12.0f) * m * (w * w + h * h);

  inertia.m[0][1] = inertia.m[0][2] =
      inertia.m[1][0] = inertia.m[1][2] =
          inertia.m[2][0] = inertia.m[2][1] = 0.0f;
}

void RigidbodyArray_Init(rigidbody_array_t* arr, size_t capacity){
  arr->capacity = capacity;
  arr->count = 0;

  // Ensures safety when capacity % SIMD_ALIGNMENT_BYTES != 0
  size_t vec_t_size = sizeof(vec_t) * capacity;
  vec_t_size = (vec_t_size + SIMD_ALIGNMENT_BYTES - 1) & ~(SIMD_ALIGNMENT_BYTES - 1);



  // position
  arr->px = ALIGNED_NEW(vec_t_size);
  arr->py = ALIGNED_NEW(vec_t_size);
  arr->pz = ALIGNED_NEW(vec_t_size);

  if (!CHECK_VALIDITY(arr->px) || !CHECK_VALIDITY(arr->py) || !CHECK_VALIDITY(arr->pz)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to position\n");
    exit(1);
  }
  
  // Velocity
  arr->vx = ALIGNED_NEW(vec_t_size);
  arr->vy = ALIGNED_NEW(vec_t_size);
  arr->vz = ALIGNED_NEW(vec_t_size);
  if (!CHECK_VALIDITY(arr->vx) || !CHECK_VALIDITY(arr->vy) || !CHECK_VALIDITY(arr->vz)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to velocity\n");
    exit(1);
  }

  // Force
  arr->fx = ALIGNED_NEW(vec_t_size);
  arr->fy = ALIGNED_NEW(vec_t_size);
  arr->fz = ALIGNED_NEW(vec_t_size);
  if (!CHECK_VALIDITY(arr->fx) || !CHECK_VALIDITY(arr->fy) || !CHECK_VALIDITY(arr->fz)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to force\n");
    exit(1);
  }
  // Torque
  arr->tx = ALIGNED_NEW(vec_t_size);
  arr->ty = ALIGNED_NEW(vec_t_size);
  arr->tz = ALIGNED_NEW(vec_t_size);
  if (!CHECK_VALIDITY(arr->tx) || !CHECK_VALIDITY(arr->ty) || !CHECK_VALIDITY(arr->tz)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to torque\n");
    exit(1);
  }
  // Angular Velocity
  arr->avx = ALIGNED_NEW(vec_t_size);
  arr->avy = ALIGNED_NEW(vec_t_size);
  arr->avz = ALIGNED_NEW(vec_t_size);
  if (!CHECK_VALIDITY(arr->avx) || !CHECK_VALIDITY(arr->avy) || !CHECK_VALIDITY(arr->avz)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to angular velocity\n");
    exit(1);
  }
  // Rotation
  arr->qx = ALIGNED_NEW(vec_t_size);
  arr->qy = ALIGNED_NEW(vec_t_size);
  arr->qz = ALIGNED_NEW(vec_t_size);
  arr->qw = ALIGNED_NEW(vec_t_size);
  if (!CHECK_VALIDITY(arr->qx) || !CHECK_VALIDITY(arr->qy) || !CHECK_VALIDITY(arr->qz) || !CHECK_VALIDITY(arr->qw)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to rotation\n");
    exit(1);
  }
  // Inertia
  arr->inv_inertia_x = ALIGNED_NEW(vec_t_size);
  arr->inv_inertia_y = ALIGNED_NEW(vec_t_size);
  arr->inv_inertia_z = ALIGNED_NEW(vec_t_size);
  if (!CHECK_VALIDITY(arr->inv_inertia_x) || !CHECK_VALIDITY(arr->inv_inertia_y) || !CHECK_VALIDITY(arr->inv_inertia_z)){
    fprintf(stderr, "[RigidbodyArray]: Failed to allocate memory to inertia\n");
    exit(1);
  }
  // Properties
  arr->mass = ALIGNED_NEW(vec_t_size);
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


void Rigidbody_Create(Vector position, Vector obb_size){
  if (gRigidbodyArray == NULL){
    fprintf(stderr, "[Rigidbody]: Attempted to create rigidbody instance -> array is NULL\n");
    exit(1);
  }
  if (gRigidbodyArray->count >= gRigidbodyArray->capacity){
    printf("[Rigidbody]: Growth not implemented");
    exit(1);
  } 
  
  rigidbody_array_t* rb = gRigidbodyArray; // Ease of typing
  size_t i = rb->count; 
  // Creation
  rb->px[i] = position.x;
  rb->py[i] = position.y;
  rb->pz[i] = position.z;
  
  
  rb->vx[i] = rb->vy[i] = rb->vz[i] = 0;
  rb->fx[i] = rb->fy[i] = rb->fz[i] = 0;
  rb->tx[i] = rb->ty[i] = rb->tz[i] = 0;
  rb->avx[i] = rb->avy[i] = rb->avz[i] = 0;

  rb->qx[i] = rb->qy[i] = rb->qz[i] = rb->qw[i] = 0;
  
  ColliderArray_AddOBB(gColliderArray, position, VectorScale(obb_size, 0.5f)); // Exits on failure
  
  mat3 inertia = {0};
  compute_box_inertia(1, obb_size, inertia);
  // Then inverse
  




}
