#include "physics/collider.h"

#include "mem.h"

void ColliderArray_Init(collider_array_dynamic_t* arr, size_t capacity){
  arr->capacity = capacity;
  arr->count = 0;
 
  size_t third_cap = (int)(capacity / 3);

  arr->count_aabb = 0; arr->capacity_aabb = third_cap;
  arr->count_obb = 0; arr->capacity_obb = third_cap;
  arr->count_sphere = 0; arr->capacity_sphere = third_cap;

  size_t vec_t_size = sizeof(Vector) * capacity;

  arr->type = malloc(sizeof(solid_t) * capacity);
  arr->rigidbody_ref = malloc(sizeof(size_t) * capacity);
  

  arr->aabb_min = ALIGNED_NEW(vec_t_size);
  arr->aabb_max = ALIGNED_NEW(vec_t_size);


  arr->obb_centre = ALIGNED_NEW(vec_t_size);
  arr->obb_half_extents = ALIGNED_NEW(vec_t_size);
  arr->obb_axis_x = ALIGNED_NEW(vec_t_size);
  arr->obb_axis_y= ALIGNED_NEW(vec_t_size);
  arr->obb_axis_z = ALIGNED_NEW(vec_t_size);

  arr->sphere_centre = ALIGNED_NEW(vec_t_size);
  arr->sphere_radius = ALIGNED_NEW(sizeof(vec_t) * capacity); 
}

void ColliderArray_Destroy(collider_array_dynamic_t* arr){
  arr->capacity = 0; arr->count = 0;

  free(arr->type); arr->type = NULL;
  free(arr->rigidbody_ref); arr->rigidbody_ref = NULL;
  free(arr->aabb_min); arr->aabb_min = NULL;
  free(arr->aabb_max); arr->aabb_max = NULL;
  free(arr->obb_centre); arr->obb_centre = NULL;
  free(arr->obb_half_extents); arr->obb_half_extents = NULL;
  free(arr->obb_axis_x); arr->obb_axis_x = NULL;
  free(arr->obb_axis_y); arr->obb_axis_y = NULL;
  free(arr->obb_axis_z); arr->obb_axis_z = NULL;

  free(arr->sphere_centre); arr->sphere_centre = NULL;
  free(arr->sphere_radius); arr->sphere_radius = NULL;
}


size_t ColliderArray_AddAABB(collider_array_dynamic_t* arr, Vector min, Vector max){
  if (arr->count_aabb >= arr->capacity_aabb){
    size_t new_cap = arr->capacity_aabb * 2;
    Vector* new_min = realloc(arr->aabb_min, sizeof(Vector) * new_cap);
    Vector* new_max = realloc(arr->aabb_max, sizeof(Vector) * new_cap);
    if (!CHECK_VALIDITY(new_min) || !CHECK_VALIDITY(new_max)){
      fprintf(stderr, "[ColliderArray]: Failed to reallocate memory to AABB array\n");
      exit(1);
    }
    arr->aabb_min = new_min;
    arr->aabb_max = new_max;
    arr->capacity_aabb = new_cap;
  }
  arr->aabb_min[arr->count_aabb] = min;
  arr->aabb_max[arr->count_aabb] = max;
  return arr->count_aabb++;
}

size_t ColliderArray_AddOBB(collider_array_dynamic_t* arr, Vector centre, Vector half_extents){
  if (arr->count_obb >= arr->capacity_obb){
    size_t new_cap = arr->capacity_obb * 2;

    Vector* centres = realloc(arr->obb_centre, sizeof(Vector) * new_cap);
    if (!CHECK_VALIDITY(centres)){ fprintf(stderr, "[ColliderArray]: Failed to reallocate memory to OBB array\n"); exit(1); }

    Vector* halfs = realloc(arr->obb_half_extents, sizeof(Vector) * new_cap);
    if (!CHECK_VALIDITY(halfs)){ fprintf(stderr, "[ColliderArray]: Failed to reallocate memory to OBB array\n"); exit(1); }

    Vector* local_x = realloc(arr->obb_axis_x, sizeof(Vector) * new_cap);
    if (!CHECK_VALIDITY(local_x)){ fprintf(stderr, "[ColliderArray]: Failed to reallocate memory to OBB array\n"); exit(1); }

    Vector* local_y = realloc(arr->obb_axis_y, sizeof(Vector) * new_cap);
    if (!CHECK_VALIDITY(local_y)){ fprintf(stderr, "[ColliderArray]: Failed to reallocate memory to OBB array\n"); exit(1); }

    Vector* local_z = realloc(arr->obb_axis_z, sizeof(Vector) * new_cap);
    if (!CHECK_VALIDITY(local_z)){ fprintf(stderr, "[ColliderArray]: Failed to reallocate memory to OBB array\n"); exit(1); }

    // Otherwise,
    arr->obb_centre = centres;
    arr->obb_half_extents = halfs;
    arr->obb_axis_x = local_x;
    arr->obb_axis_y = local_y;
    arr->obb_axis_z = local_z;

    arr->capacity_obb = new_cap;
  }
  size_t current = arr->count_obb;
  arr->obb_centre[current] = centre;
  arr->obb_half_extents[current] = half_extents;
  arr->obb_axis_x[current] = VECTOR_AXIS_X;
  arr->obb_axis_y[current] = VECTOR_AXIS_Y;
  arr->obb_axis_z[current] = VECTOR_AXIS_Z;
  return current++;
}

// Collision detection
Vector aabb_compute_centre(Vector min, Vector max){
  return VectorScale(
    VectorAdd(min, max), 0.5f
  );
}

Vector aabb_compute_halfs(Vector min, Vector max){
  return VectorScale(
    VectorSub(max, min), 0.5f
  );
}

int collisiondetect_aabb_plane(Vector min, Vector max, clip_plane_t plane){
  Vector normal = VectorCrossNormalise(plane.tangent, plane.bittangent);
  Vector centre = aabb_compute_centre(min, max);

  float distance = VectorDot(
    normal, VectorSub(centre, plane.centre)
  );

  Vector halfs = aabb_compute_halfs(min, max);

  float r = halfs.x * fabsf(normal.x) + halfs.y * fabsf(normal.y) + halfs.z * fabsf(normal.z);

  if (fabsf(distance) > r){
    return false;
  }

  return true;
}
