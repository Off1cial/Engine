#ifndef BSP_H
#define BSP_H

#include "editor/brush.h"
#include "types/types_vector.h"

// BSP Elements

typedef struct {
  plane_t plane; // split plane
  int front; // index of child nodes, -1 if it has none (node is a leaf)
  int back;
} bsp_node_t;

typedef struct {
  contents_t contents;
  int face_start; // Index of first face
  int face_count;
} bsp_leaf_t;

typedef struct {
  winding_t winding;
  int material;
  Vector2 uv_origin;
  Vector uv_axis_u;
  Vector uv_axis_v;
} bsp_face_t;



// Contained within
typedef struct 
{
  bsp_node_t* nodes;
  bsp_leaf_t* leaves;
  bsp_face_t* faces;

  size_t leaf_count, leaf_capacity;
  size_t node_count, node_capacity;
  size_t face_count, face_capacity;
  
} bsp_tree_t;

bsp_tree_t* BSP_Compile(void);
bool BSP_IsSolid(bsp_tree_t* tree, Vector point);


// Temporary debug
extern plane_t BSP_DEBUG_SPLITPLANES[4096];
extern mesh_t* BSP_DEBUG_SPLITPLANES_MESHES[4096];
extern int split_plane_count;


#endif