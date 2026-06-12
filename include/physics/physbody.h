#pragma once

#include "types/types_vector.h"

typedef struct{
  Vector* vertices;
  int vertex_count;
  unsigned int* indices;
  int index_count;
} cmesh_t;


typedef struct {
  union {
    struct {float hx, hy, hz;} aabb;
    struct {float radius;} sphere;
    struct {cmesh_t mesh;} mesh;
  };
} collider_info_t; 


typedef enum {
  PHYSCOLLIDER_AABB,
  PHYSCOLLIDER_SPHERE,
  PHYSCOLLIDER_CAPSULE,
  PHYSCOLLIDER_MESH
} collider_type_t;

typedef struct {

  // Position
  float* x;
  float* y;
  float* z;
  // Velocity
  float* vx;
  float* vy;
  float* vz;
  // Acceleration
  float* ax;
  float* ay;
  float* az;
  // Force
  float* fx;
  float* fy;
  float* fz;

  float* inv_mass;
   
  int* grounded;

  // Shape data
  collider_type_t* ctype;
  float* radius; // sphere/capsule
  float* height; // capsule height

  float* halfx, *halfy, *halfz;
  int* mesh_index;

  // cold data
  size_t count;
  size_t capacity;

} physbody_array_t;

extern physbody_array_t* gPhysbodyArray;
void PhysbodyArray_Init(size_t capacity);



int Physbody_AddAABB(Vector pos, Vector halfExtents, float mass);
int Physbody_AddSphere(Vector pos, float radius, float mass);
int Physbody_AddCapsule(Vector pos, float radius, float height, float mass);
int Physbody_AddMesh(Vector pos, int meshIndex, float mass);



void PhysbodyArray_Step(float dt, Vector gravity);