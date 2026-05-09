#include "editor/brush.h"

#include <immintrin.h> // AVX
#include <stddef.h>
#include "mem.h"
#include <string.h>
#include <stdalign.h>
#include "rendering/shader.h"
#include "rendering/mesh.h"
#include "rendering/render_commands.h"
#include "rendering/draw_list.h"
#include "rendering/camera.h"

#ifndef EPSILON
#define EPSILON 1e-6
#endif

int EditorBrushArray_Init(brush_array_t *arr, size_t initial_capacity)
{
  if (!arr)
  {
    fprintf(stderr, "[BrushArray]: NULL pointer passed to init\n");
    return 0;
  }

  if (initial_capacity == 0)
    initial_capacity = 8;

  memset(arr, 0, sizeof(*arr));

  arr->brush_capacity = initial_capacity;
  arr->brush_count = 0;
  arr->total_sides = 0;

  size_t cap = arr->brush_capacity;

  // ---- Allocate transform SoA ----
  arr->px = ALIGNED_NEW(sizeof(vec_t) * cap);
  arr->py = ALIGNED_NEW(sizeof(vec_t) * cap);
  arr->pz = ALIGNED_NEW(sizeof(vec_t) * cap);

  arr->sx = ALIGNED_NEW(sizeof(vec_t) * cap);
  arr->sy = ALIGNED_NEW(sizeof(vec_t) * cap);
  arr->sz = ALIGNED_NEW(sizeof(vec_t) * cap);

  arr->qx = ALIGNED_NEW(sizeof(vec_t) * cap);
  arr->qy = ALIGNED_NEW(sizeof(vec_t) * cap);
  arr->qz = ALIGNED_NEW(sizeof(vec_t) * cap);
  arr->qw = ALIGNED_NEW(sizeof(vec_t) * cap);

  arr->side_count = ALIGNED_NEW(sizeof(size_t) * cap);
  arr->side_start = ALIGNED_NEW(sizeof(size_t) * cap);

  // Worst-case sides allocation
  arr->sides = ALIGNED_NEW(sizeof(brush_side_t) * cap * MAX_BRUSH_FACES);

  arr->editor_meshes = ALIGNED_NEW(sizeof(mesh_t *) * cap);

  // ---- Validate allocations ----
  if (!CHECK_VALIDITY(arr->px) || !CHECK_VALIDITY(arr->py) || !CHECK_VALIDITY(arr->pz) ||
      !CHECK_VALIDITY(arr->sx) || !CHECK_VALIDITY(arr->sy) || !CHECK_VALIDITY(arr->sz) ||
      !CHECK_VALIDITY(arr->qx) || !CHECK_VALIDITY(arr->qy) ||
      !CHECK_VALIDITY(arr->qz) || !CHECK_VALIDITY(arr->qw) ||
      !CHECK_VALIDITY(arr->side_count) || !CHECK_VALIDITY(arr->side_start) ||
      !CHECK_VALIDITY(arr->sides))
  {
    fprintf(stderr, "[BrushArray]: Allocation failure\n");
    return 0;
  }

  // ---- Zero memory for deterministic behaviour ----
  memset(arr->px, 0, sizeof(vec_t) * cap);
  memset(arr->py, 0, sizeof(vec_t) * cap);
  memset(arr->pz, 0, sizeof(vec_t) * cap);

  memset(arr->sx, 0, sizeof(vec_t) * cap);
  memset(arr->sy, 0, sizeof(vec_t) * cap);
  memset(arr->sz, 0, sizeof(vec_t) * cap);

  memset(arr->qx, 0, sizeof(vec_t) * cap);
  memset(arr->qy, 0, sizeof(vec_t) * cap);
  memset(arr->qz, 0, sizeof(vec_t) * cap);
  memset(arr->qw, 0, sizeof(vec_t) * cap);

  memset(arr->side_count, 0, sizeof(size_t) * cap);
  memset(arr->side_start, 0, sizeof(size_t) * cap);

  memset(arr->sides, 0, sizeof(brush_side_t) * cap * MAX_BRUSH_FACES);
  memset(arr->editor_meshes, 0, sizeof(mesh_t *) * cap);

  return 1;
}

void EditorBrushArray_Destroy(brush_array_t *arr)
{
  if (!arr)
    return;

  ALIGNED_FREE(arr->px);
  ALIGNED_FREE(arr->py);
  ALIGNED_FREE(arr->pz);

  ALIGNED_FREE(arr->sx);
  ALIGNED_FREE(arr->sy);
  ALIGNED_FREE(arr->sz);

  ALIGNED_FREE(arr->qx);
  ALIGNED_FREE(arr->qy);
  ALIGNED_FREE(arr->qz);
  ALIGNED_FREE(arr->qw);

  ALIGNED_FREE(arr->side_count);
  ALIGNED_FREE(arr->side_start);
  ALIGNED_FREE(arr->sides);
  ALIGNED_FREE(arr->editor_meshes);

  memset(arr, 0, sizeof(*arr));
}

/*
static inline aligned_block_t CREATE_ALIGNED_BLOCK(void *ptr, size_t size_bytes)
{
  aligned_block_t blk;
  blk.ptr = ptr;
  blk.size = size_bytes;
  return blk;
}
*/

static void grow_brush_array(brush_array_t *arr)
{
  if (!arr)
    return;

  size_t old_capacity = arr->brush_capacity;
  size_t new_capacity = old_capacity ? old_capacity * 2 : 8;

  // ---- positions ----
  arr->px = ALIGNED_REALLOC(arr->px,
                            old_capacity * sizeof(vec_t),
                            new_capacity * sizeof(vec_t));

  arr->py = ALIGNED_REALLOC(arr->py,
                            old_capacity * sizeof(vec_t),
                            new_capacity * sizeof(vec_t));

  arr->pz = ALIGNED_REALLOC(arr->pz,
                            old_capacity * sizeof(vec_t),
                            new_capacity * sizeof(vec_t));

  // ---- scale ----
  arr->sx = ALIGNED_REALLOC(arr->sx,
                            old_capacity * sizeof(vec_t),
                            new_capacity * sizeof(vec_t));

  arr->sy = ALIGNED_REALLOC(arr->sy,
                            old_capacity * sizeof(vec_t),
                            new_capacity * sizeof(vec_t));

  arr->sz = ALIGNED_REALLOC(arr->sz,
                            old_capacity * sizeof(vec_t),
                            new_capacity * sizeof(vec_t));

  // ---- rotation ----
  arr->qx = ALIGNED_REALLOC(arr->qx,
                            old_capacity * sizeof(vec_t),
                            new_capacity * sizeof(vec_t));

  arr->qy = ALIGNED_REALLOC(arr->qy,
                            old_capacity * sizeof(vec_t),
                            new_capacity * sizeof(vec_t));

  arr->qz = ALIGNED_REALLOC(arr->qz,
                            old_capacity * sizeof(vec_t),
                            new_capacity * sizeof(vec_t));

  arr->qw = ALIGNED_REALLOC(arr->qw,
                            old_capacity * sizeof(vec_t),
                            new_capacity * sizeof(vec_t));

  // ---- side data ----
  arr->side_count = ALIGNED_REALLOC(arr->side_count,
                                    old_capacity * sizeof(size_t),
                                    new_capacity * sizeof(size_t));

  arr->side_start = ALIGNED_REALLOC(arr->side_start,
                                    old_capacity * sizeof(size_t),
                                    new_capacity * sizeof(size_t));

  // ---- meshes ----
  arr->editor_meshes = ALIGNED_REALLOC(arr->editor_meshes,
                                       old_capacity * sizeof(mesh_t *),
                                       new_capacity * sizeof(mesh_t *));

  // update capacity
  arr->brush_capacity = new_capacity;
}

mat4 create_brush_model_matrix(brush_array_t *arr, size_t brush)
{
  mat4 scale = Mat4Identity();
  scale.m[0][0] = arr->sx[brush];
  scale.m[1][1] = arr->sy[brush];
  scale.m[2][2] = arr->sz[brush];

  mat4 rotation = QuaternionToMat4(arr->qx[brush], arr->qy[brush], arr->qz[brush], arr->qw[brush]);

  mat4 translation = Mat4Identity();
  translation.m[3][0] = arr->px[brush];
  translation.m[3][1] = arr->py[brush];
  translation.m[3][2] = arr->pz[brush];

  mat4 RS = Mat4Mul(&rotation, &scale);
  mat4 model = Mat4Mul(&translation, &RS);

  return model;
}

void EditorBrush_DrawAll(brush_array_t *arr, rdrawqueue_t *q, struct mem_arena_t *arena, camera_t *camera)
{
  for (size_t b = 0; b < arr->brush_count; b++)
  {
    if (arr->editor_meshes[b] == NULL)
    {
      printf("[RENDER]: Skipping null brush mesh\n");
      continue;
    }

    // Allocate on arena

    struct rcmd_t *cmd = MEM_ARENA_ALLOC(
        arena, sizeof(struct rcmd_t),
        alignof(struct rcmd_t));
    // Prepare command
    // Assumes default shader, no material at this point in time
    cmd->type = RCMD_DRAW_MESH;
    cmd->draw_mesh.mesh = arr->editor_meshes[b];
    cmd->draw_mesh.shader = SHADER_default_shader;
    //cmd->draw_mesh.view = camera->view;
    //cmd->draw_mesh.projection = camera->projection;
    cmd->draw_mesh.model = create_brush_model_matrix(arr, b);
    // cmd->draw_mesh.model = Mat4Identity();
    cmd->draw_mesh.material_index = 0;
    cmd->draw_mesh.mode = GL_LINE_LOOP;

    RDrawQueue_Push(q, cmd);
  }
}

void EditorBrush_Create(brush_array_t *arr, Vector position, Vector scale)
{

  if (arr->brush_count >= arr->brush_capacity)
  {
    grow_brush_array(arr);
  }

  size_t i = arr->brush_count;

  // Position
  arr->px[i] = position.x;
  arr->py[i] = position.y;
  arr->pz[i] = position.z;
  // Scale
  arr->sx[i] = scale.x;
  arr->sy[i] = scale.y;
  arr->sz[i] = scale.z;
  // Quaternion
  arr->qx[i] = 0;
  arr->qy[i] = 0;
  arr->qz[i] = 0;
  arr->qw[i] = 1;
  // Sides
  arr->side_start[i] = arr->total_sides;
  arr->side_count[i] = 6;
  for (int s = 0; s < 6; s++)
  {
    size_t side = s + arr->total_sides;
    //arr->sides[side] = create_unit_cube_side(s, position);
    arr->sides[side].brush_owner = i;
    arr->sides[side].material = 0;
  }

  printf("[Editor]: Brush Created\n");
  printf("  Position: ");
  Vector_DPrint(&position);

  arr->total_sides += 6;
  arr->editor_meshes[i] = MESH_PRIMITIVES[MESH_PRIMITIVE_CUBE];
  arr->brush_count++;
}
