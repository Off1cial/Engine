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

static void get_unit_cube_face_vertices(Vector out[4], eAXIS axis, Vector centre){
  Vector fCentre = VectorAdd(centre, VectorScale(VECTOR_AXES[axis], 0.5f) ); 

} 

static brush_side_t create_unit_cube_side(eAXIS axis, Vector centre){
  brush_side_t side;


  switch(axis){
    case X_POS:{
      float dist = centre.x + 0.5f;
      Vector p_normal = (Vector){1, 0, 0};

    }
  }
  
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
  arr->side_count[i] = 6;
  arr->total_sides += 6;
  // return i?
  arr->brush_count++;
}
