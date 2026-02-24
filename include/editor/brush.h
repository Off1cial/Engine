#ifndef EDITOR_BRUSH_H
#define EDITOR_BRUSH_H

// glShadeModel(GL_FLAT)???

#include <stdint.h>
#include <stdlib.h>
#include "types/types_vector.h"

#define MAX_BRUSH_FACES 64
#define MAX_WINDING_POINTS 64

typedef struct {
  Vector normal;
  vec_t dist;
} plane_t;

typedef struct{
  size_t count;
  Vector points[MAX_WINDING_POINTS];
} winding_t;

typedef struct {
  plane_t plane;
  winding_t* winding;
  int material;

} brush_side_t;


typedef struct {
  Vector vertices[MAX_WINDING_POINTS];
  size_t v_count;
  size_t planenum;
  int material;
} face_t;


// EDITOR

typedef struct {
  size_t brush_count;
  size_t brush_capacity;
  // brush faces
  size_t total_sides;
  size_t* side_count;
  size_t* side_start; // Index of first side
  // Position
  vec_t* px;
  vec_t* py;
  vec_t* pz;
  // Size
  vec_t* sx;
  vec_t* sy;
  vec_t* sz;
  // Quaternion rotation
  vec_t* qx;
  vec_t* qy;
  vec_t* qz;
  vec_t* qw;

  brush_side_t* sides;
  
} brush_array_t;

int clip_winding_against_plane(const winding_t* in, winding_t* out, const plane_t plane);


void EditorBrush_Create(brush_array_t* arr, Vector position);

#endif
