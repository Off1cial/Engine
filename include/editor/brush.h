#ifndef EDITOR_BRUSH_H
#define EDITOR_BRUSH_H

// glShadeModel(GL_FLAT)???

#include <stdint.h>
#include <stdlib.h>
#include "types/types_vector.h"
#include "rendering/camera.h"
#include "rendering/mesh.h"
#include "rendering/draw_list.h"
#include "rendering/renderer.h"
#include "mem.h"

#define MAX_BRUSH_FACES 64
#define MAX_BRUSHES 2048
#define MAX_WINDING_POINTS 64
#define BRUSH_DEFUALT_SCALE 2
#define MAX_BRUSH_EDGES 256

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
  Vector a, b;

  int side_a;
  int side_b; // optional -> -1
  int brush; // Index to gEditorBrushArray->brushes
} brush_edge_t;




typedef struct brush_t {
  brush_side_t sides[MAX_BRUSH_FACES]; // Local space to centre
  brush_edge_t edges[MAX_BRUSH_EDGES];
  int edge_count;

  int side_count;
  int dirty;
  int dirty_ui_edges;
  mesh_t editor_mesh;

  Vector pos, rot, scale;

} brush_t;

typedef enum {
  BRUSH_HANDLE_SIDE,
  BRUSH_HANDLE_POSITION,
  BRUSH_HANDLE_ROTATE
} brush_handle_type_t;

typedef struct {

  brush_handle_type_t type;

  union{
    // Used to drag brush edges
    struct {
      Vector2 positon; // GUI Position
      brush_edge_t* edge;
      float delta; // Distance to move along normal
    } side_handle;
    // Used to drag brushes by their centre
    struct {
      Vector2 position; // GUI Position
      brush_t* brush;
      Vector2 delta;
    } pos_handle;
    // Used to rotate brushes around their centre
    struct {
      Vector2 position; // GUI Position
      brush_t* brush;
      float delta_2d; // Radians to rotate brush by in GUI
      Vector delta_3d; // Radians to rotate by each axis in 3D
    } rotate_handle;
  };

} brush_handle_t;







typedef struct {
  brush_side_t* side;
  brush_t* owner_brush;
  material_t* material;
  mesh_t mesh;

  int dirty; // Does mesh need recomputing?

} brush_side_hovered_t;


// Editor UI

#define EDITOR_UI_BRUSH_SIDE_WIDTH_PX 8




typedef struct {
  brush_side_hovered_t hovered_side;
  brush_handle_t handles[128];
  /*
  32 is more than enough - 
  clicking on a plane in one panel will give one handle for each edge,
  one for position which can be interchanged for rotation depending on tool selected
  */
  brush_t* brushes;
  size_t count, capacity;
  size_t handle_count;
} editor_brush_array;

extern editor_brush_array* gEditorBrushArray;


void EditorBrushArray_Init(editor_brush_array* arr, size_t capacity);
void EditorBrushArray_Destroy(editor_brush_array* arr);

void BrushHoveredSideComputeMesh(brush_side_hovered_t* hside);
void BrushToMesh(brush_t *b, mesh_t* mesh_out);
brush_t make_brush_cube(Vector mins, Vector maxs);

void EditorBrush_Draw(brush_t* brush, rdrawqueue_t* drawlist, camera_t* cam);
void EditorBrush_DrawHoveredSide(brush_side_hovered_t* hside, bool print);
bool Brush_Raycast(brush_t* brush, int* out_side, Vector* out_hit, float* out_dist, camera_t* camera, float cursorx, float cursory);

static winding_t base_winding(plane_t p)
{
  winding_t w = {0};

  Vector up = (fabsf(p.normal.y) < 0.9f) ? VECTOR_AXIS_Y : VECTOR_AXIS_X;

  Vector u = VectorCrossNormalise(up, p.normal);
  Vector v = VectorCrossNormalise(p.normal, u);

  float size = 1000.0f;

  Vector centre = VectorScale(p.normal, p.dist);

  Vector au = VectorScale(u, size);
  Vector av = VectorScale(v, size);

  w.v[0] = VectorAdd(VectorAdd(centre, au), av);
  w.v[1] = VectorSub(VectorAdd(centre, au), av);
  w.v[2] = VectorSub(VectorSub(centre, au), av);
  w.v[3] = VectorAdd(VectorSub(centre, au), av);

  w.count = 4;
  return w;
}

static winding_t clip_winding(winding_t *in, plane_t p)
{
  winding_t out = {0};

  for (int i = 0; i < in->count; i++)
  {
    Vector a = in->v[i];
    Vector b = in->v[(i + 1) % in->count];

    float da = VectorDot(p.normal, a) - p.dist;
    float db = VectorDot(p.normal, b) - p.dist;

    int ina = (da <= 0);
    int inb = (db <= 0);

    if (ina)
      out.v[out.count++] = a;

    if (ina != inb)
    {
      float t = da / (da - db);
      Vector hit = VectorAdd(a, VectorScale(VectorSub(b, a), t));
      out.v[out.count++] = hit;
    }
  }

  return out;
}


#endif
