#ifndef PHYSICS_RIGIDBODY_H
#define PHYSICS_RIGIDBODY_H

#include "types/types_vector.h"
#include <stdint.h>
#include <stdlib.h>


typedef struct {
  
  size_t count, capacity;

  // Position
  vec_t* px;
  vec_t* py;
  vec_t* pz;
  // Velocity
  vec_t* vx;
  vec_t* vy;
  vec_t* vz;
  // Force
  vec_t* fx;
  vec_t* fy;
  vec_t* fz;
  // Torque
  vec_t* tx;
  vec_t* ty;
  vec_t* tz;
  // Angular Velcoity
  vec_t* avx;
  vec_t* avy;
  vec_t* avz;
  // Rotation
  vec_t* qx;
  vec_t* qy;
  vec_t* qz;
  vec_t* qw;
  // Inertia
  vec_t* inv_inertia_x;
  vec_t* inv_inertia_y;
  vec_t* inv_inertia_z;
  // Properties
  vec_t* mass;
  vec_t* inv_mass;
 

  uint8_t* rotation_lock;

} rigidbody_array_t;

extern rigidbody_array_t* gRigidbodyArray;

void RigidbodyArray_Init(rigidbody_array_t* arr, size_t capacity);
void RigidbodyArray_Destroy(rigidbody_array_t* arr);
void Rigidbody_Create(Vector position, Vector obb_size);

#endif
