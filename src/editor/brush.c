#include "editor/brush.h"

#include <immintrin.h> // AVX
#include <stddef.h>
#include "mem.h"

#ifndef EPSILON
#define EPSILON 1e-6
#endif

int clip_winding_against_plane(const winding_t* in, winding_t* out, const plane_t plane){
  if (!in || !out || in->count < 3){ return 1; }

  out->count = 0;


  for (size_t p = 0; p < in->count; p++){
    Vector a = in->points[p];
    Vector b = in->points[ (p+1) % in->count ];
    
    Vector AB = VectorSub(b, a);

    float den = VectorDot(AB, plane.normal);

    int a_inside = ( VectorDot(a, plane.normal) - plane.dist <= 0 );
    int b_inside = ( VectorDot(b, plane.normal) - plane.dist <= 0 );
    
    if (fabsf(den) < EPSILON){
      // Edge parallel to plane
      if (a_inside){
        out->points[out->count++] = b;
      }
    }


    if (a_inside && b_inside){
      out->points[out->count++] = b;
    }
    // Inside -> Outside
    if (a_inside && !b_inside){
      // Find intersection
      double t =
        ( plane.dist - VectorDot(plane.normal, a) ) /
        VectorDot(plane.normal, AB);
      out->points[out->count++] = VectorAdd( a,  VectorScale(AB, t) );
    }

    // Outside -> inside
    if (!a_inside && b_inside){
      double t =
        ( plane.dist - VectorDot(plane.normal, a) ) /
        VectorDot(plane.normal, AB);
      out->points[out->count++] = VectorAdd( a,  VectorScale(AB, t) );
      out->points[out->count++] = b;
    }
  }
  return (out->count >= 3);
}



int EditorBrushArray_Init(brush_array_t* arr, size_t initial_capacity)
{
    if (!arr) {
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
    arr->px = ALIGNED_NEW(sizeof(vec_t) * cap).ptr;
    arr->py = ALIGNED_NEW(sizeof(vec_t) * cap).ptr;
    arr->pz = ALIGNED_NEW(sizeof(vec_t) * cap).ptr;

    arr->sx = ALIGNED_NEW(sizeof(vec_t) * cap).ptr;
    arr->sy = ALIGNED_NEW(sizeof(vec_t) * cap).ptr;
    arr->sz = ALIGNED_NEW(sizeof(vec_t) * cap).ptr;

    arr->qx = ALIGNED_NEW(sizeof(vec_t) * cap).ptr;
    arr->qy = ALIGNED_NEW(sizeof(vec_t) * cap).ptr;
    arr->qz = ALIGNED_NEW(sizeof(vec_t) * cap).ptr;
    arr->qw = ALIGNED_NEW(sizeof(vec_t) * cap).ptr;

    arr->side_count = ALIGNED_NEW(sizeof(size_t) * cap).ptr;
    arr->side_start = ALIGNED_NEW(sizeof(size_t) * cap).ptr;

    // Worst-case sides allocation
    arr->sides = ALIGNED_NEW(sizeof(brush_side_t) * cap * MAX_BRUSH_FACES).ptr;

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

    return 1;
}


void EditorBrushArray_Destroy(brush_array_t* arr)
{
    if (!arr) return;

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

    memset(arr, 0, sizeof(*arr));
}



static inline aligned_block_t CREATE_ALIGNED_BLOCK(void* ptr, size_t size_bytes) {
    aligned_block_t blk;
    blk.ptr = ptr;
    blk.size = size_bytes;
    return blk;
}

static void grow_brush_array(brush_array_t* arr) {
    if (!arr) return;
    size_t old_capacity = arr->brush_capacity;
    size_t new_capacity = old_capacity ? old_capacity * 2 : 8; // start with 8 if empty

    // --- Reallocate positions ---
    aligned_block_t px_blk = { arr->px, old_capacity * sizeof(vec_t) };
    px_blk = ALIGNED_REALLOC(px_blk, new_capacity * sizeof(vec_t));
    arr->px = (vec_t*)px_blk.ptr;

    aligned_block_t py_blk = { arr->py, old_capacity * sizeof(vec_t) };
    py_blk = ALIGNED_REALLOC(py_blk, new_capacity * sizeof(vec_t));
    arr->py = (vec_t*)py_blk.ptr;

    aligned_block_t pz_blk = { arr->pz, old_capacity * sizeof(vec_t) };
    pz_blk = ALIGNED_REALLOC(pz_blk, new_capacity * sizeof(vec_t));
    arr->pz = (vec_t*)pz_blk.ptr;

    // --- Reallocate sizes ---
    aligned_block_t sx_blk = { arr->sx, old_capacity * sizeof(vec_t) };
    sx_blk = ALIGNED_REALLOC(sx_blk, new_capacity * sizeof(vec_t));
    arr->sx = (vec_t*)sx_blk.ptr;

    aligned_block_t sy_blk = { arr->sy, old_capacity * sizeof(vec_t) };
    sy_blk = ALIGNED_REALLOC(sy_blk, new_capacity * sizeof(vec_t));
    arr->sy = (vec_t*)sy_blk.ptr;

    aligned_block_t sz_blk = { arr->sz, old_capacity * sizeof(vec_t) };
    sz_blk = ALIGNED_REALLOC(sz_blk, new_capacity * sizeof(vec_t));
    arr->sz = (vec_t*)sz_blk.ptr;

    // --- Reallocate quaternion rotation ---
    aligned_block_t qx_blk = { arr->qx, old_capacity * sizeof(vec_t) };
    qx_blk = ALIGNED_REALLOC(qx_blk, new_capacity * sizeof(vec_t));
    arr->qx = (vec_t*)qx_blk.ptr;

    aligned_block_t qy_blk = { arr->qy, old_capacity * sizeof(vec_t) };
    qy_blk = ALIGNED_REALLOC(qy_blk, new_capacity * sizeof(vec_t));
    arr->qy = (vec_t*)qy_blk.ptr;

    aligned_block_t qz_blk = { arr->qz, old_capacity * sizeof(vec_t) };
    qz_blk = ALIGNED_REALLOC(qz_blk, new_capacity * sizeof(vec_t));
    arr->qz = (vec_t*)qz_blk.ptr;

    aligned_block_t qw_blk = { arr->qw, old_capacity * sizeof(vec_t) };
    qw_blk = ALIGNED_REALLOC(qw_blk, new_capacity * sizeof(vec_t));
    arr->qw = (vec_t*)qw_blk.ptr;

    // --- Reallocate side count ---
    aligned_block_t side_count_blk = { arr->side_count, old_capacity * sizeof(size_t) };
    side_count_blk = ALIGNED_REALLOC(side_count_blk, new_capacity * sizeof(size_t));
    arr->side_count = (size_t*)side_count_blk.ptr;

    // --- Reallocate side start ---
    aligned_block_t side_start_blk = { arr->side_start, old_capacity * sizeof(size_t) };
    side_start_blk = ALIGNED_REALLOC(side_start_blk, new_capacity * sizeof(size_t));
    arr->side_start = (size_t*)side_start_blk.ptr;

    // Update capacity
    arr->brush_capacity = new_capacity;
}


static Vector get_axis_u(Vector normal){
  if (abs(normal.x == 1)) return VECTOR_AXIS_Y;
  if (abs(normal.y == 1)) return VECTOR_AXIS_X;
  return VECTOR_AXIS_X;
}

static Vector get_axis_v(Vector normal){
  if (abs(normal.x == 1)) return VECTOR_AXIS_Z;
  if (abs(normal.y == 1)) return VECTOR_AXIS_Z;
  return VECTOR_AXIS_Y;
}

static brush_side_t create_unit_cube_side(eAXIS axis, Vector centre){
  brush_side_t side = {0};
  side.winding = ALIGNED_NEW(sizeof(winding_t)).ptr;
  // Remember to free the above when destroying brushes or converting to .bsp
  if (!side.winding){
    fprintf(stderr, "[Brush]: Failed to allocate winding\n");
    exit(1);
  }
  side.winding->count = 0;
  plane_t plane;
  plane.normal = VECTOR_AXES[axis];
  plane.dist = VectorDot(plane.normal, VectorAdd(centre, VectorScale(plane.normal, 0.5)));

  float half = 0.5f;  
  
  Vector u = get_axis_u(VECTOR_AXES[axis]);
  Vector v = get_axis_v(VECTOR_AXES[axis]);

  for (int i = -1; i <= 1; i+=2){
    for (int j = -1; j <= 1; j+=2){
      Vector a = VectorScale(u, i * half);    
      Vector b = VectorScale(v, j * half);
      Vector tmp = VectorAdd(a, b);
      side.winding->points[side.winding->count++] = VectorAdd(centre, tmp);
    }
  }
  return side;  
}

void EditorBrush_Create(brush_array_t* arr, Vector position){
  // All brushes start as axis-aligned unit cubes
  if (arr->brush_count >= arr->brush_capacity){
    grow_brush_array(arr);
  }

  // TODO If I ever wanted to spawn 1 goygillion brushes at once, use SIMD
  size_t i = arr->brush_count;
  arr->px[i] = position.x;
  arr->py[i] = position.y;
  arr->pz[i] = position.z;
  
  arr->sx[i] = 1;
  arr->sy[i] = 1;
  arr->sz[i] = 1;

  arr->qx[i] = 0;
  arr->qy[i] = 0;
  arr->qz[i] = 0;
  arr->qw[i] = 1;
  

  // Create sides
  
  arr->side_start[i] = arr->total_sides;
  size_t start = arr->side_start[i];
  for (int s = 0; s < 6; s++){
    arr->sides[s + start] = create_unit_cube_side(s, position);
  }


  arr->side_count[i] = 6;
  arr->total_sides += 6;
  // return i?
  arr->brush_count++;
}
