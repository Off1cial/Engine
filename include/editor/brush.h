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
#define MAX_BRUSHES 2048
#define MAX_WINDING_POINTS 64

typedef struct {
  Vector v[MAX_WINDING_POINTS];
  int count;
} winding_t;


typedef struct {
  plane_t plane;

  int material_id;   // texture reference
  Vector2 uv_origin;
  Vector uv_axis_u;
  Vector uv_axis_v;

} brush_side_t;

typedef struct {
  brush_side_t sides[MAX_BRUSH_FACES]; // Local space to centre
  int side_count;
  int dirty;
  mesh_t editor_mesh;

  Vector pos, rot, scale;


} brush_t;

typedef struct {
  brush_t* brushes;
  size_t count, capacity;
} editor_brush_array;

extern editor_brush_array* gEditorBrushArray;


void EditorBrushArray_Init(editor_brush_array* arr, size_t capacity);
void EditorBrushArray_Destroy(editor_brush_array* arr);

mesh_t BrushToMesh(brush_t *b);
brush_t make_brush_cube(Vector mins, Vector maxs);

void EditorBrush_Draw(brush_t* brush, rdrawqueue_t* drawlist, camera_t* cam);
bool Brush_Raycast(brush_t* brush, int* out_side, Vector* out_hit, float* out_dist, camera_t* camera, float cursorx, float cursory);

#endif