#ifndef PHYSICS_COLLIDER_H
#define PHYSICS_COLLIDER_H

#include <stdlib.h>
#include <stdint.h>
#include "types/types_vector.h"

typedef enum { SOLID_AABB, SOLID_OBB, SOLID_PLANE, SOLID_SPHERE } solid_t;

typedef struct {

  size_t count, capacity;

  solid_t* type;
  size_t* rigidbody_ref;

  // AABB
  size_t count_aabb, capacity_aabb;
  Vector* aabb_min; 
  Vector* aabb_max;
  // OBB
  size_t count_obb, capacity_obb;
  Vector* obb_centre;
  Vector* obb_half_extents;
  Vector* obb_axis_x;
  Vector* obb_axis_y;
  Vector* obb_axis_z;
  // Sphere
  size_t count_sphere, capacity_sphere;
  Vector* sphere_centre;
  vec_t* sphere_radius;

} collider_array_dynamic_t;

void ColliderArray_Init(collider_array_dynamic_t* arr, size_t capacity);
void ColliderArray_Destroy(collider_array_dynamic_t* arr);

size_t ColliderArray_AddAABB(collider_array_dynamic_t* arr, Vector min, Vector max);
size_t ColliderArray_AddOBB(collider_array_dynamic_t* arr, Vector centre, Vector half_extents); // OBB start aligned to Y axis
size_t ColliderArray_AddSphere(collider_array_dynamic_t* arr, Vector centre, float radius);

#endif
