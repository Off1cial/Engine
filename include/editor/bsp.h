#ifndef BSP_H
#define BSP_H

#include "types/types_vector.h"
#include "editor/brush.h"

typedef struct
{
  plane_t plane;

  int front;
  int back;

} bsp_node_t;

typedef struct
{
  Vector verts[MAX_WINDING_POINTS];
  Vector2 uvs[MAX_WINDING_POINTS];

  int material_id;
} bsp_face_t;


typedef struct
{
  int contents; // flag
} bsp_leaf_t;

typedef struct
{
  bsp_node_t *nodes;
  bsp_leaf_t* leaves;
  bsp_face_t *faces;

  int leaf_count;
  int leaf_capacity;
  int node_count;
  int node_capacity;
  int face_count;
} bsp_container_t;
#endif