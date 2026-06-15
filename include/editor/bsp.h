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

/*
typedef struct {
  winding_t winding;
  int material;
  Vector2 uv_origin;
  Vector uv_axis_u;
  Vector uv_axis_v;
} bsp_face_t;
*/

// CURRENTLY REWRITING THE FILE, THE 4 BELOW ARE NEW
typedef enum epside {
  PSIDE_FRONT    = 1 << 0, // 1
  PSIDE_BACK     = 1 << 1, // 2
  PSIDE_COPLANAR = 1 << 2  // 4
} planeside_t;

typedef struct {
  //brush_side_t* original;
  winding_t win;
  int planenum;
  int fcontents, bcontents;
} bspside_t;

typedef struct{
  brush_t* original;
  int sidecount;
  bspside_t sides[];
} bspbrush_t;

typedef struct{
  int planenum;
  int front, back;
} node_t;


typedef struct face_t {
  winding_t win;
  Vector2 uvs[MAX_WINDING_POINTS];
  int material_id;
  int contents_front;
  int contents_back;
  Vector debug_colour;
  Vector tangent; // For UV/Normals
  Vector bit_tangent; // For UV/Normals
  plane_t plane;
} face_t;


// Contained within
typedef struct 
{
  bsp_node_t* nodes;
  bsp_leaf_t* leaves;
  face_t* faces;

  size_t leaf_count, leaf_capacity;
  size_t node_count, node_capacity;
  size_t face_count, face_capacity;
  
} bsp_tree_t;

bsp_tree_t* BSP_Compile(void);
bsp_tree_t *BSP2_Compile(void);
bool BSP_IsSolid(bsp_tree_t* tree, Vector point);

void R_DrawBSPFaces(camera_t* camera, face_t* faces, int face_count);
void R_DrawBSPPlanes();

extern bsp_tree_t* BSP_ACTIVE_TREE;

// Temporary debug
extern plane_t BSP_DEBUG_SPLITPLANES[4096];

extern int split_plane_count;



#endif