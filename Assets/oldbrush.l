#ifndef EDITOR_BRUSH_H
#define EDITOR_BRUSH_H

// glShadeModel(GL_FLAT)???

#include <stdint.h>
#include <stdlib.h>
#include "types/types_vector.h"
#include "rendering/camera.h"
#include "rendering/mesh.h"
#include "rendering/draw_list.h"
#include "mem.h"

#define MAX_BRUSH_FACES 64
#define MAX_WINDING_POINTS 64



// In-Editor
typedef struct {
  plane_t plane;
  int material;
  int brush_owner;
} brush_side_t;



// EDITOR

typedef struct {
  size_t brush_count;
  size_t brush_capacity;
  // brush faces
  size_t total_sides;
  long int hovered_side; // Index of side under cursor
  long int hovered_brush;
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

  mesh_t** editor_meshes;
  
} brush_array_t;


#ifdef __cplusplus
extern "C" {
#endif

extern brush_array_t* gEditorBrushArray;

int EditorBrushArray_Init(brush_array_t* arr, size_t initial_capacity);
void EditorBrushArray_Destroy(brush_array_t* arr);

void EditorBrush_Create(brush_array_t* arr, Vector position, Vector scale);
void EditorBrush_DrawAll(brush_array_t* arr, rdrawqueue_t* q, struct mem_arena_t* arena, camera_t* camera);
void EditorBrush_RebuildBrush(brush_array_t* arr, size_t brush);
void EditorBrush_RebuildAll(brush_array_t* arr);



#ifdef __cplusplus
}
#endif

#endif
