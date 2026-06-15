#include "editor/bsp.h"
#include "types/types_vector.h"
#include <assert.h>
#include <float.h>
#include <stdint.h>
#include <stdlib.h>
#include "mem.h"

// This must be the 5th rewrite of this file - 08/06/2026 03:03,
// trying to keep it tidy to stop me from chimping out

// #define BSP_CONT_CAPACITY_NODE_INIT 64
// #define BSP_CONT_CAPACITY_LEAF_INIT 64
// #define BSP_CONT_CAPACITY_FACE_INIT 128

#define MAX_BSP_NODES 1024
#define MAX_BSP_LEAVES 1024
#define MAX_TREE_DEPTH 64
#define MAX_BRUSHES_AT_LEVEL 4096

#define MAX_CANDIDATES (512)
#define LEAF(i) (-((i) + 1))
#define LEAF_IDX(v) (-(v) - 1)
#define IS_LEAF(v) ((v) < 0)

#define BSP_CANDIDATE_STRADDLE_PENALTY 2.0F
#define BSP_CANDIDATE_AXIAL_BONUS 0.5F

Vector axis_colours[6] = {
    {1, 0, 0}, // +X  red
    {0, 1, 0}, // +Y  green
    {0, 0, 1}, // +Z  blue
    {1, 0, 1}, // -X  magenta
    {1, 1, 0}, // -Y  yellow
    {0, 1, 1}  // -Z  cyan
};

// Temporary debug purposes

plane_t BSP_DEBUG_SPLITPLANES[4096];
mesh_t *BSP_DEBUG_SPLITPLANES_MESHES[4096];
size_t BSP_DEBUG_SPLITPLANES_COUNT = 0;
int gNodeCount = 0;
int gLeafCount = 0;
int gLeafSolid = 0;
int gLeafEmpty = 0;

#include "rendering/render_commands.h"
#include "rendering/mesh.h"
// very inefficient and rough but it works for debugging
mesh_t *FACE_MESHES[10240];
size_t face_mesh_count = 0;

static mesh_t *face_to_mesh(face_t *face)
{
  if (!face)
    return NULL;
  WindingEnsureOrientation(&face->win, PlaneReverse(face->plane).normal);
  mesh_t *mesh = malloc(sizeof(mesh_t));
  MeshInit(mesh, face->win.count, face->win.count - 2);

  // Push all vertices first
  GLuint indices[MAX_WINDING_POINTS];
  for (int i = 0; i < face->win.count; i++)
  {
    struct vertex_t vert = {
        .pos = face->win.v[i],
        .colour = face->debug_colour,
        .uv = face->uvs[i],
        .tangent = face->tangent};
    indices[i] = MeshPushVertex(mesh, vert);
  }

  // Fan triangulation using the indices
  for (int i = 1; i < face->win.count - 1; i++)
  {
    MeshPushTriangle(mesh, indices[0], indices[i], indices[i + 1]);
  }

  MeshUpload(mesh, GL_STATIC_DRAW);
  return mesh;
}
static void mesh_array_fill(bsp_tree_t *tree)
{
  for (int i = 0; i < tree->face_count; i++)
  {

    FACE_MESHES[face_mesh_count++] = face_to_mesh(&tree->faces[i]);
    // printf("Face mesh added\n");
  }
}

void R_DrawBSPFaces(camera_t *camera, face_t *faces, int face_count)
{
  for (int i = 0; i < face_mesh_count; i++)
  {
    float dot = VectorDot(camera->pos, faces[i].plane.normal) - faces[i].plane.dist;
    if (dot < 0)
    {
      continue;
    }
    struct rcmd_t *rcmd = MEM_ARENA_ALLOC(gMemArena, sizeof(struct rcmd_t), alignof(struct rcmd_t));
    rcmd->type = RCMD_DRAW_MESH;
    rcmd->draw_mesh.mesh = FACE_MESHES[i];
    rcmd->draw_mesh.model = Mat4Identity();
    rcmd->draw_mesh.mode = GL_TRIANGLES;
    rcmd->draw_mesh.material = gRendererState->materials[0];
    rcmd->draw_mesh.wireframe = false;
    rcmd->draw_mesh.use_colour_override = false;
    RDrawQueue_Push(gRendererState->draw_q, rcmd);
  }
}

void R_DrawBSPPlanes()
{
  for (int i = 0; i < BSP_DEBUG_SPLITPLANES_COUNT; i++)
  {
    struct rcmd_t *rcmd = MEM_ARENA_ALLOC(gMemArena, sizeof(struct rcmd_t), alignof(struct rcmd_t));
    rcmd->type = RCMD_DRAW_MESH;
    rcmd->draw_mesh.mesh = BSP_DEBUG_SPLITPLANES_MESHES[i];
    rcmd->draw_mesh.material = gRendererState->materials[2];
    rcmd->draw_mesh.mode = GL_TRIANGLES;
    rcmd->draw_mesh.model = Mat4Identity();
    rcmd->draw_mesh.wireframe = false;
    rcmd->draw_mesh.use_colour_override = false;
    RDrawQueue_Push(gRendererState->draw_q, rcmd);
  }
}

typedef enum
{
  SPLIT_FRONT = 1,
  SPLIT_BACK = 2,
  SPLIT_BOTH = 3
} split_result_t;

typedef struct
{
  plane_t plane;
  float score;
} candidate_t;

static int plane_equal(plane_t a, plane_t b)
{
  return VectorMag(VectorSub(a.normal, b.normal)) < 0.001f &&
         fabsf(a.dist - b.dist) < 0.01f;
}

static int check_plane_dupe(plane_t *planes, size_t n, plane_t p)
{
  for (size_t i = 0; i < n; i++)
  {
    if (plane_equal(p, planes[i]))
      return 1; // Dupes found
  }
  return 0; // No dupes found
}

static bool point_in_brush(brush_t *brush, Vector p)
{
  if (!brush)
    return false;
  for (int s = 0; s < brush->side_count; s++)
  {
    brush_side_t *side = &brush->sides[s];
    float dist = VectorDot(p, side->plane.normal) - side->plane.dist;
    if (dist > EPSILON)
    {
      return false;
    }
  }
  return true;
}

static void bsp_container_init(bsp_tree_t *cont)
{
  assert(cont);

  cont->node_count = 0;
  cont->leaf_count = 0;
  cont->face_count = 0;

  cont->node_capacity = MAX_BSP_NODES;
  cont->leaf_capacity = MAX_BSP_LEAVES;
  cont->face_capacity = MAX_BRUSH_FACES * MAX_BRUSHES;
  // Should be MAX_BRUSH_FACES * MAX_BRUSHES

  cont->nodes = malloc(sizeof(bsp_node_t) * cont->node_capacity);
  cont->leaves = malloc(sizeof(bsp_leaf_t) * cont->leaf_capacity);
  cont->faces = malloc(sizeof(face_t) * cont->face_capacity);

  assert(cont->nodes);
  assert(cont->leaves);
  assert(cont->faces);
}

static void bsp_container_reset(bsp_tree_t *cont)
{
  assert(cont);
  cont->node_count = 0;
  cont->node_capacity = 0;

  cont->leaf_count = 0;
  cont->leaf_capacity = 0;

  cont->face_count = 0;
  cont->face_capacity = 0;
}

static void bsp_container_free(bsp_tree_t *cont)
{
  assert(cont);

  bsp_container_reset(cont);

  if (cont->nodes)
    free(cont->nodes);
  if (cont->leaves)
    free(cont->leaves);
  if (cont->faces)
    free(cont->faces);
}

static winding_t build_brush_face(brush_t *in, int side)
{
  if (!in)
    return (winding_t){0};

  winding_t face = base_winding(in->sides[side].plane);

  for (int i = 0; i < in->side_count; i++)
  {
    if (i == side)
      continue;
    face = clip_winding(&face, in->sides[i].plane);
    if (face.count < 3)
      break;
  }
  WindingEnsureOrientation(&face, in->sides[side].plane.normal);

  return face;
}

/*
static split_result_t split_brush(brush_t *in, plane_t s_plane, brush_t **front, brush_t **back)
{
  *front = NULL, *back = NULL;

  brush_t *f = calloc(1, sizeof(brush_t));
  brush_t *b = calloc(1, sizeof(brush_t));
  assert(f);
  assert(b);

  f->contents = in->contents;
  b->contents = in->contents;

  winding_t w_plane = base_winding(s_plane);
  if (WINDING_DEGEN(w_plane))
    return 0;

  // Split each side against the split plane to determine which sides belong on the front or back brush
  for (int i = 0; i < in->side_count; i++)
  {
    brush_side_t *side = &in->sides[i];
    winding_t face = build_brush_face(in, i);
    if (WINDING_DEGEN(face))
      continue;

    // Stop faces leaking to both sides
    if (plane_equal(s_plane, side->plane))
    {
      // Add to back brush
      if (b->side_count < MAX_BRUSH_FACES)
      {
        b->sides[b->side_count++] = *side;
      }
      continue;
    }
    if (plane_equal(PlaneReverse(s_plane), side->plane))
    {
      // Add to front brush
      if (f->side_count < MAX_BRUSH_FACES)
      {
        f->sides[f->side_count++] = *side;
      }
    }

    // Which parts of the side are in front or behind the plane
    winding_t wfront = clip_winding(&face, PlaneReverse(s_plane));
    winding_t wback = clip_winding(&face, s_plane);

    // Add the sides that maintain validity to the brush, not the winding - geometry computed later
    // This exactly how my brushes are made
    if (WINDING_VALID(wfront) && f->side_count < MAX_BRUSH_FACES)
    {
      f->sides[f->side_count++] = *side;
    }
    if (WINDING_VALID(wback) && b->side_count < MAX_BRUSH_FACES)
    {
      b->sides[b->side_count++] = *side;
    }
  }

  // Add the split plane to the brushes to seal the splice
  if (f->side_count > 0 && f->side_count < MAX_BRUSH_FACES)
  {
    f->sides[f->side_count].plane = PlaneReverse(s_plane);
    f->sides[f->side_count].material_id = -1;
    f->side_count++;
    // UVs, material?
  }
  if (b->side_count > 0 && b->side_count < MAX_BRUSH_FACES)
  {
    b->sides[b->side_count].plane = s_plane;
    b->sides[b->side_count].material_id = -1;
    b->side_count++;
  }

  split_result_t result = 0;

  // What is the condition for a valid brush?
  if (f->side_count >= 4)
  {
    *front = f;
    result |= SPLIT_FRONT;
  }
  else
  {
    free(f);
  }
  if (b->side_count >= 4)
  {
    *back = b;
    result |= SPLIT_BACK;
  }
  else
  {
    free(b);
  }
  return result;
}
*/

static split_result_t split_brush(brush_t *in, plane_t s_plane, brush_t **front, brush_t **back)
{
    *front = *back = NULL;

    // Quick check: if the brush lies entirely on one side, just copy it
    bool has_front = false, has_back = false;
    for (int i = 0; i < in->side_count && !(has_front && has_back); i++) {
        winding_t w = build_brush_face(in, i);
        if (w.count < 3) continue;
        for (int j = 0; j < w.count; j++) {
            float d = VectorDot(s_plane.normal, w.v[j]) - s_plane.dist;
            if (d >  EPSILON) has_front = true;
            if (d < -EPSILON) has_back  = true;
        }
    }
    if (!has_back) {
        *front = malloc(sizeof(brush_t));
        memcpy(*front, in, sizeof(brush_t));
        return SPLIT_FRONT;
    }
    if (!has_front) {
        *back = malloc(sizeof(brush_t));
        memcpy(*back, in, sizeof(brush_t));
        return SPLIT_BACK;
    }

    brush_t *f = calloc(1, sizeof(brush_t));
    brush_t *b = calloc(1, sizeof(brush_t));
    f->contents = in->contents;
    b->contents = in->contents;

    // Build split‑plane winding and clip it by all brush planes
    winding_t mid = base_winding(s_plane);
    for (int i = 0; i < in->side_count && mid.count >= 3; i++) {
        if (plane_equal(s_plane, in->sides[i].plane) ||
            plane_equal(PlaneReverse(s_plane), in->sides[i].plane))
            continue;
        mid = clip_winding(&mid, in->sides[i].plane);
    }
    if (mid.count < 3) {
        // Split plane doesn't intersect the brush – put brush on dominant side
        free(f); free(b);
        if (has_front) {
            *front = malloc(sizeof(brush_t));
            memcpy(*front, in, sizeof(brush_t));
            return SPLIT_FRONT;
        } else {
            *back = malloc(sizeof(brush_t));
            memcpy(*back, in, sizeof(brush_t));
            return SPLIT_BACK;
        }
    }

    // Clip each side of the original brush
    for (int i = 0; i < in->side_count; i++) {
        winding_t w = build_brush_face(in, i);
        if (w.count < 3) continue;

        winding_t wfront = clip_winding(&w, PlaneReverse(s_plane));
        winding_t wback  = clip_winding(&w, s_plane);

        if (wfront.count >= 3 && f->side_count < MAX_BRUSH_FACES) {
            f->sides[f->side_count] = in->sides[i];
            f->side_count++;
        }
        if (wback.count >= 3 && b->side_count < MAX_BRUSH_FACES) {
            b->sides[b->side_count] = in->sides[i];
            b->side_count++;
        }
    }

    // Seal the cut
    if (f->side_count > 0 && f->side_count < MAX_BRUSH_FACES) {
        f->sides[f->side_count].plane = PlaneReverse(s_plane);
        f->sides[f->side_count].material_id = -1;
        f->side_count++;
    }
    if (b->side_count > 0 && b->side_count < MAX_BRUSH_FACES) {
        b->sides[b->side_count].plane = s_plane;
        b->sides[b->side_count].material_id = -1;
        b->side_count++;
    }

    // Validate and return
    split_result_t result = 0;
    if (f->side_count >= 4) { *front = f; result |= SPLIT_FRONT; }
    else { free(f); *front = NULL; }
    if (b->side_count >= 4) { *back  = b; result |= SPLIT_BACK; }
    else { free(b); *back = NULL; }

    if (!*front && !*back) {
        if (has_front) {
            *front = malloc(sizeof(brush_t));
            memcpy(*front, in, sizeof(brush_t));
            return SPLIT_FRONT;
        } else {
            *back = malloc(sizeof(brush_t));
            memcpy(*back, in, sizeof(brush_t));
            return SPLIT_BACK;
        }
    }
    return result;
}

static plane_t choose_splane(brush_t **brushes, size_t brush_count)
{
  candidate_t candidates[MAX_CANDIDATES];
  int cand_count = 0;

  // Gather candidates by iterating brush sides
  for (int i = 0; i < brush_count; i++)
  {
    for (int j = 0; j < brushes[i]->side_count; j++)
    {
      brush_side_t *side = &brushes[i]->sides[j];

      // is this plane already present?
      int dupe = 0;
      // iterate candidates
      for (int c = 0; c < cand_count; c++)
      {
        if (plane_equal(candidates[c].plane, side->plane))
        {
          dupe = 1;
          break;
        }
      }
      if (dupe)
        continue;

      // add plane to candidates
      if (!dupe && cand_count < MAX_CANDIDATES)
      {
        candidates[cand_count].plane = side->plane;
        candidates[cand_count].score = 0.0f;
        cand_count++;
      }
    }
  }

  // Fallback plane if no candidates gathered
  if (cand_count <= 0)
  {
    plane_t fallback = (plane_t){.dist = 0.0f, .normal = VECTOR_AXIS_Y};
    return fallback;
    // probably shit but oh well
  }

  // score the planes
  for (int c = 0; c < cand_count; c++)
  {
    // award balance, and axial normals
    // penalise straddles
    int front_count = 0;
    int back_count = 0;
    int straddle_count = 0;

    // Test by splitting each plane by this brush - expensive
    // This loop gathers the amount of brushes infront, behind and straddling the plane separately
    for (int i = 0; i < brush_count; i++)
    {
      brush_t *f = NULL, *b = NULL;
      split_result_t result = split_brush(
          brushes[i],
          candidates[c].plane,
          &f,
          &b);

      if (result == SPLIT_BOTH)
      {
        straddle_count++;
      }
      else if (result == SPLIT_FRONT)
      {
        front_count++;
      }
      else if (result == SPLIT_BACK)
      {
        back_count++;
      }
      if (f)
        free(f);
      if (b)
        free(b);
    }
    // Calculate balance, penalise great imbalance
    if (front_count == 0 && straddle_count == 0)
    {
      candidates[c].score = -FLT_MAX;
      continue;
    }
    if (back_count == 0 && straddle_count == 0)
    {
      candidates[c].score = -FLT_MAX;
      continue;
    }

    // balance : 1.0 - perfect balance. 0.0 - shit, all on one side
    float balance = 1.0f -
                    fabsf(
                        (float)(front_count - back_count) /
                        (float)(front_count + back_count + 1));
    float straddle_penalty = straddle_count * BSP_CANDIDATE_STRADDLE_PENALTY;
    float axial = 0.0f;
    if (
        fabsf(candidates[c].plane.normal.x) > 0.99f ||
        fabsf(candidates[c].plane.normal.y) > 0.99f ||
        fabsf(candidates[c].plane.normal.z) > 0.99f)
    {
      axial = BSP_CANDIDATE_AXIAL_BONUS;
    }
    candidates[c].score = balance * 10.0f - straddle_penalty + axial;
    /*
    printf("  plane: normal(%.1f, %.1f, %.1f) dist=%.1f -> score=%.2f (f=%d b=%d x=%d)\n",
           candidates[c].plane.normal.x,
           candidates[c].plane.normal.y,
           candidates[c].plane.normal.z,
           candidates[c].plane.dist,
           candidates[c].score,
           front_count,
           back_count,
           straddle_count);
    */
  }

  // Return best plane
  int best = 0;
  for (int c = 0; c < cand_count; c++)
  {
    if (candidates[c].score > candidates[best].score)
    {
      best = c;
    }
  }
  return candidates[best].plane;
}

/*
static int build_tree(brush_t **brushes, int brush_count, bsp_tree_t *tree, int depth)
{
  // Exit cases
  if (depth >= MAX_TREE_DEPTH || tree->node_count >= MAX_BSP_NODES || tree->leaf_count >= MAX_BSP_LEAVES)
  {
    int idx = tree->leaf_count++;
    tree->leaves[idx].contents = (brush_count > 0) ? brushes[0]->contents : CONTENTS_EMPTY;
    tree->leaves[idx].face_start = -1;
    tree->leaves[idx].face_count = 0;
    return LEAF(idx);
  }

  if (brush_count == 0)
  {
    int idx = tree->leaf_count++;
    tree->leaves[idx].contents = CONTENTS_EMPTY;
    tree->leaves[idx].face_start = -1;
    tree->leaves[idx].face_count = 0;
    return LEAF(idx);
  }

  // Pick split plane
  plane_t split = choose_splane(brushes, brush_count);

  // Debug: track unique planes
  if (!check_plane_dupe(BSP_DEBUG_SPLITPLANES, BSP_DEBUG_SPLITPLANES_COUNT, split))
    BSP_DEBUG_SPLITPLANES[BSP_DEBUG_SPLITPLANES_COUNT++] = split;

  // Split brushes
  brush_t **fronts = malloc(sizeof(brush_t *) * MAX_BRUSHES_AT_LEVEL);
  brush_t **backs = malloc(sizeof(brush_t *) * MAX_BRUSHES_AT_LEVEL);
  int front_count = 0, back_count = 0;

  for (int i = 0; i < brush_count; i++)
  {
    brush_t *f = NULL, *b = NULL;
    split_result_t res = split_brush(brushes[i], split, &f, &b);

    if ((res & SPLIT_FRONT) && f && front_count < MAX_BRUSHES_AT_LEVEL)
      fronts[front_count++] = f;
    else if (f)
      free(f);

    if ((res & SPLIT_BACK) && b && back_count < MAX_BRUSHES_AT_LEVEL)
      backs[back_count++] = b;
    else if (b)
      free(b);
  }

  // If the split didn't separate anything, make a leaf
  if (front_count == 0 || back_count == 0)
  {
    printf(
        "[BSP] Failed split depth=%d f=%d b=%d plane=(%.1f %.1f %.1f %.1f)\n",
        depth,
        front_count,
        back_count,
        split.normal.x,
        split.normal.y,
        split.normal.z,
        split.dist);

    printf(
        "depth=%d brush_count=%d f=%d b=%d\n",
        depth,
        brush_count,
        front_count,
        back_count);

    for (int i = 0; i < front_count; i++)
      free(fronts[i]);
    for (int i = 0; i < back_count; i++)
      free(backs[i]);

    // Determine leaf contents by sampling the brush's center
    int contents = CONTENTS_EMPTY;
    for (int i = 0; i < brush_count; i++)
    {
      if (point_in_brush(brushes[i], brushes[i]->pos))
      {
        contents = brushes[i]->contents;
        break;
      }
    }

    int idx = tree->leaf_count++;
    tree->leaves[idx].contents = contents;
    tree->leaves[idx].face_start = -1;
    tree->leaves[idx].face_count = 0;
    return LEAF(idx);
  }

  // Create node and recurse
  int node_idx = tree->node_count++;
  tree->nodes[node_idx].plane = split;

  tree->nodes[node_idx].front = build_tree(fronts, front_count, tree, depth + 1);
  tree->nodes[node_idx].back = build_tree(backs, back_count, tree, depth + 1);

  for (int i = 0; i < front_count; i++)
    free(fronts[i]);
  for (int i = 0; i < back_count; i++)
    free(backs[i]);

  free(fronts);
  free(backs);
  return node_idx;
}
*/

static int build_tree(brush_t **brushes, int brush_count, bsp_tree_t *tree, int depth)
{
    // Exit cases
    if (depth >= MAX_TREE_DEPTH || tree->node_count >= MAX_BSP_NODES || tree->leaf_count >= MAX_BSP_LEAVES)
    {
        int idx = tree->leaf_count++;
        // Use a point guaranteed to be inside the brush
        int contents = CONTENTS_EMPTY;
        if (brush_count > 0) {
            winding_t w = build_brush_face(brushes[0], 0);
            if (w.count >= 3) {
                Vector inside = {0,0,0};
                for (int j = 0; j < w.count; j++)
                    inside = VectorAdd(inside, w.v[j]);
                inside = VectorScale(inside, 1.0f / w.count);
                if (point_in_brush(brushes[0], inside))
                    contents = brushes[0]->contents;
            }
        }
        tree->leaves[idx].contents = contents;
        tree->leaves[idx].face_start = -1;
        tree->leaves[idx].face_count = 0;
        return LEAF(idx);
    }

    if (brush_count == 0)
    {
        int idx = tree->leaf_count++;
        tree->leaves[idx].contents = CONTENTS_EMPTY;
        tree->leaves[idx].face_start = -1;
        tree->leaves[idx].face_count = 0;
        return LEAF(idx);
    }

    // Pick split plane
    plane_t split = choose_splane(brushes, brush_count);

    // Debug: track unique planes
    if (!check_plane_dupe(BSP_DEBUG_SPLITPLANES, BSP_DEBUG_SPLITPLANES_COUNT, split))
        BSP_DEBUG_SPLITPLANES[BSP_DEBUG_SPLITPLANES_COUNT++] = split;

    // Split brushes
    brush_t **fronts = malloc(sizeof(brush_t *) * MAX_BRUSHES_AT_LEVEL);
    brush_t **backs  = malloc(sizeof(brush_t *) * MAX_BRUSHES_AT_LEVEL);
    int front_count = 0, back_count = 0;

    for (int i = 0; i < brush_count; i++)
    {
        brush_t *f = NULL, *b = NULL;
        split_result_t res = split_brush(brushes[i], split, &f, &b);

        if ((res & SPLIT_FRONT) && f && front_count < MAX_BRUSHES_AT_LEVEL)
            fronts[front_count++] = f;
        else if (f)
            free(f);

        if ((res & SPLIT_BACK) && b && back_count < MAX_BRUSHES_AT_LEVEL)
            backs[back_count++] = b;
        else if (b)
            free(b);
    }
    // Always create a node – empty children become empty leaves
    if (front_count == 0 || back_count == 0) {
        int node_idx = tree->node_count++;
        tree->nodes[node_idx].plane = split;

        if (front_count == 0) {
            tree->nodes[node_idx].front = build_tree(NULL, 0, tree, depth + 1);  // empty leaf
            tree->nodes[node_idx].back  = build_tree(backs, back_count, tree, depth + 1);
        } else {
            tree->nodes[node_idx].front = build_tree(fronts, front_count, tree, depth + 1);
            tree->nodes[node_idx].back  = build_tree(NULL, 0, tree, depth + 1);  // empty leaf
        }

        // Clean up temporary arrays
        for (int i = 0; i < front_count; i++) free(fronts[i]);
        for (int i = 0; i < back_count; i++)  free(backs[i]);
        free(fronts);
        free(backs);
        return node_idx;
    }


    // Create node and recurse
    int node_idx = tree->node_count++;
    tree->nodes[node_idx].plane = split;

    tree->nodes[node_idx].front = build_tree(fronts, front_count, tree, depth + 1);
    tree->nodes[node_idx].back = build_tree(backs, back_count, tree, depth + 1);

    for (int i = 0; i < front_count; i++)
        free(fronts[i]);
    for (int i = 0; i < back_count; i++)
        free(backs[i]);

    free(fronts);
    free(backs);
    return node_idx;
}

int BSP_FindLeaf(bsp_tree_t *tree, Vector point)
{
  if (!tree)
    return -1;
  int current = 0;
  while (!IS_LEAF(current))
  {

    bsp_node_t *node = &tree->nodes[current];
    float d = VectorDot(point, node->plane.normal) - node->plane.dist;
    current = (d >= 0) ? node->front : node->back;
  }
  return LEAF_IDX(current);
}

bool BSP_IsSolid(bsp_tree_t *tree, Vector point)
{
  int leaf = BSP_FindLeaf(tree, point);
  if (leaf < 0 || leaf >= tree->leaf_count)
    return false;
  return tree->leaves[leaf].contents == CONTENTS_SOLID;
}

static void add_face_to_tree(bsp_tree_t *tree, face_t face)
{
  if (tree->face_count >= tree->face_capacity)
  {
    fprintf(stderr, "[BSP]: Cannot add face to tree, exceeds limit\n");
    exit(1);
  }
  tree->faces[tree->face_count++] = face;
}

static bool BoundsOverlap(Vector mins1, Vector maxs1, Vector mins2, Vector maxs2)
{
  return (mins1.x <= maxs2.x && maxs1.x >= mins2.x &&
          mins1.y <= maxs2.y && maxs1.y >= mins2.y &&
          mins1.z <= maxs2.z && maxs1.z >= mins2.z);
}

// Compute face bounding box from winding
static void FaceBounds(winding_t *w, Vector *mins, Vector *maxs)
{
  *mins = *maxs = w->v[0];
  for (int i = 1; i < w->count; i++)
  {
    if (w->v[i].x < mins->x)
      mins->x = w->v[i].x;
    if (w->v[i].y < mins->y)
      mins->y = w->v[i].y;
    if (w->v[i].z < mins->z)
      mins->z = w->v[i].z;
    if (w->v[i].x > maxs->x)
      maxs->x = w->v[i].x;
    if (w->v[i].y > maxs->y)
      maxs->y = w->v[i].y;
    if (w->v[i].z > maxs->z)
      maxs->z = w->v[i].z;
  }
}

static void ComputePlanarUVs(winding_t *w, plane_t plane, Vector2 *uvs_out, float scale)
{
  // Build basis vectors on the plane
  Vector u_axis, v_axis;
  Vector up = {0, 1, 0};
  if (fabsf(plane.normal.y) > 0.99f)
    up = (Vector){0, 0, 1};

  u_axis = VectorCrossNormalise(up, plane.normal);
  v_axis = VectorCrossNormalise(plane.normal, u_axis);

  for (int i = 0; i < w->count; i++)
  {
    uvs_out[i].x = VectorDot(w->v[i], u_axis) * scale;
    uvs_out[i].y = VectorDot(w->v[i], v_axis) * scale;
  }
}

// REWRITE STARTS

bspbrush_t *AllocBrush(brush_t *b)
{
  int numsides = b->side_count;
  bspbrush_t *bb;
  size_t s;
  s = (size_t)(sizeof(bspbrush_t) + numsides * sizeof(bspside_t));
  bb = malloc(s);
  memset(bb, 0, s);
  bb->original = b;
  bb->sidecount = numsides;
  return bb;
}

// Returns a heap pointer to a base winding on plane p
/*
winding_t* AllocWinding(plane_t p){
  winding_t* w = malloc(sizeof(winding_t));
  memset(w, 0, sizeof(winding_t));

  Vector up = (fabsf(p.normal.y) < 0.9f) ? VECTOR_AXIS_Y : VECTOR_AXIS_X;

  Vector u = VectorCrossNormalise(up, p.normal);
  Vector v = VectorCrossNormalise(p.normal, u);

  float size = 1000.0f;

  Vector centre = VectorScale(p.normal, p.dist);

  Vector au = VectorScale(u, size);
  Vector av = VectorScale(v, size);

  w->v[0] = VectorAdd(VectorAdd(centre, au), av);
  w->v[1] = VectorSub(VectorAdd(centre, au), av);
  w->v[2] = VectorSub(VectorSub(centre, au), av);
  w->v[3] = VectorAdd(VectorSub(centre, au), av);

  w->count = 4;
  return w;
}

*/

void FreeBrush(bspbrush_t *bb)
{
  bb->original = NULL;
  bb->sidecount = 0;
  free(bb);
}

plane_t *gBSPplanes = NULL;
int gBSPplanecount = 0;
int gBSPplanecapacity = 128;

bspside_t *gBSPsides = NULL;
int gBSPsidecount = 0;
int gBSPsidecapacity = 128;

static void plane_array_init()
{
  gBSPplanes = malloc(sizeof(plane_t) * gBSPplanecapacity);
}

static int plane_array_add(plane_t p)
{
  for (int i = 0; i < gBSPplanecount; i++)
  {
    if (plane_equal(p, gBSPplanes[i]))
    {
      printf("Planenum return = %d\n", i);
      return i;
    }
  }

  if (gBSPplanecount >= gBSPplanecapacity)
  {
    gBSPplanecapacity *= 2;
    gBSPplanes = realloc(gBSPplanes, sizeof(plane_t) * gBSPplanecapacity);
  }

  int idx = gBSPplanecount;
  gBSPplanes[idx] = p;
  gBSPplanecount++;
  printf("Planenum return = %d\n", idx);
  return idx;
}

/*
static winding_t* build_side(brush_t* brush, int side){
  brush_side_t* s = &brush->sides[side];

  winding_t* win = AllocWinding(s->plane);
  for (int i = 0; i < brush->side_count; i++){
    if (i == side)
      continue;
    winding_t clipped = clip_winding(win, brush->sides[i].plane);
    *win = clipped;
  }
  return win;
}
*/

static void side_array_fill()
{
  gBSPsides = (bspside_t *)malloc(sizeof(bspside_t) * gBSPsidecapacity);

  for (int b = 0; b < gEditorBrushArray->count; b++)
  {
    brush_t *brush = &gEditorBrushArray->brushes[b];
    if (brush->is_entity)
      continue;
    for (int s = 0; s < brush->side_count; s++)
    {
      // printf("SIDE PLANE DIST = %f\n", brush->sides[s].plane.dist);
      if (gBSPsidecount >= gBSPsidecapacity)
      {
        gBSPsidecapacity *= 2;
        gBSPsides = realloc(gBSPsides, sizeof(bspside_t) * gBSPsidecapacity);
      }
      int pnum = plane_array_add(brush->sides[s].plane);
      // printf("PNUM=%d\n", pnum);
      gBSPsides[gBSPsidecount++] =
          (bspside_t){
              .win = build_brush_face(brush, s),
              .planenum = pnum,
              .fcontents = CONTENTS_EMPTY,
              .bcontents = brush->contents};
    }
  }
}

static planeside_t classify_side(bspside_t *side, int planenum)
{
  plane_t *p = &gBSPplanes[planenum];
  winding_t *win = &side->win;
  planeside_t result = 0;
  for (int i = 0; i < side->win.count; i++)
  {
    float d = VectorDot(p->normal, win->v[i]) - p->dist;
    if (d < -EPSILON)
      result |= PSIDE_BACK;
    if (d > EPSILON)
      result |= PSIDE_FRONT;
  }
  return result;
}

static int print_side_array_debug(bspside_t *sides, int count)
{
  printf("{ ");
  for (int i = 0; i < count; i++)
  {
    printf("%d ", sides[i].planenum);
  }
  printf(" }\n");
}

static void split_side(bspside_t *in, int planenum, bspside_t *front, bspside_t *back)
{
  winding_t wfront = {0};
  winding_t wback = {0};

  wfront = clip_winding(&in->win, PlaneReverse(gBSPplanes[planenum]));
  wback = clip_winding(&in->win, gBSPplanes[planenum]);
  front->win = wfront;
  back->win = wback;
}

static int ChooseSplitPlane(bspside_t *sides, int sidecount)
{
  // Gather unique planes
  int unique_planes[256]; // or smaller, but safe
  int n_unique = 0;
  for (int i = 0; i < sidecount; i++)
  {
    int p = sides[i].planenum;
    int found = 0;
    for (int j = 0; j < n_unique; j++)
    {
      if (unique_planes[j] == p)
      {
        found = 1;
        break;
      }
    }
    if (!found)
      unique_planes[n_unique++] = p;
  }

  int best_plane = sides[0].planenum;
  int best_score = INT_MAX; // lower is better (fewer straddles, balanced)

  for (int u = 0; u < n_unique; u++)
  {
    int front = 0, back = 0, straddle = 0;
    for (int i = 0; i < sidecount; i++)
    {
      planeside_t res = classify_side(&sides[i], unique_planes[u]);
      if (res == 0)
      {
        front++;
      } // coplanar → arbitrarily front
      else if ((res & PSIDE_FRONT) && (res & PSIDE_BACK))
      {
        straddle++;
      }
      else if (res & PSIDE_FRONT)
      {
        front++;
      }
      else if (res & PSIDE_BACK)
      {
        back++;
      }
    }

    // Reject planes that don't split anything
    if (front == 0 || back == 0)
      continue;

    int balance_score = abs(front - back);
    int straddle_penalty = straddle * 9; // heavily penalise straddles
    int total = balance_score + straddle_penalty;
    if (total < best_score)
    {
      best_score = total;
      best_plane = unique_planes[u];
    }
  }

  return best_plane;
}

// malloc one array for the entire bsp, use a pointer to represent depth?
static int BSP_BuildTree_r(bsp_tree_t *tree, bspside_t *sides, int sidecount, int depth)
{
  printf("DEPTH %d", depth);
  print_side_array_debug(sides, sidecount);

  int base_case = (tree->node_count >= MAX_BSP_NODES ||
                   tree->leaf_count >= MAX_BSP_LEAVES ||
                   depth >= MAX_TREE_DEPTH ||
                   sidecount == 0);

  if (base_case)
  {
    int idx = tree->leaf_count++;
    tree->leaves[idx].contents = (sidecount > 0) ? sides[0].bcontents : CONTENTS_EMPTY;
    tree->leaves[idx].face_start = -1;
    tree->leaves[idx].face_count = 0;
    if (tree->leaves[idx].contents == CONTENTS_EMPTY)
      gLeafEmpty++;
    else
      gLeafSolid++;
    return LEAF(idx);
  }

  int splitplane = ChooseSplitPlane(sides, sidecount);
  assert(splitplane >= 0);
  assert(splitplane < gBSPplanecount);
  printf("SPLITPLANE=%d\n", splitplane);
  printf("depth=%d sidecount=%d\n", depth, sidecount);
  if (!check_plane_dupe(BSP_DEBUG_SPLITPLANES, BSP_DEBUG_SPLITPLANES_COUNT, gBSPplanes[splitplane]))
  {
    BSP_DEBUG_SPLITPLANES[BSP_DEBUG_SPLITPLANES_COUNT++] = gBSPplanes[splitplane];
  }

  int frontcount = 0, backcount = 0;
  bspside_t *fronts = malloc(sizeof(bspside_t) * MAX_BRUSHES_AT_LEVEL);
  bspside_t *backs = malloc(sizeof(bspside_t) * MAX_BRUSHES_AT_LEVEL);
  assert(fronts);
  assert(backs);
  memset(fronts, 0, sizeof(bspside_t) * MAX_BRUSHES_AT_LEVEL);
  memset(backs, 0, sizeof(bspside_t) * MAX_BRUSHES_AT_LEVEL);
  for (int i = 0; i < sidecount; i++)
  {

    // printf("SIDES[i]:\n  pnum=%d\n  fcontents=%d\n  bcontents=%d\n", sides[i].planenum, sides[i].fcontents, sides[i].bcontents);
    planeside_t result = classify_side(&sides[i], splitplane);
    if (result == 0)
    {
      fronts[frontcount++] = sides[i];
      continue;
    }
    if (result == (PSIDE_FRONT | PSIDE_BACK))
    {

      bspside_t front = {0};
      bspside_t back = {0};

      front.planenum = sides[i].planenum;
      back.planenum = sides[i].planenum;
      front.fcontents = sides[i].fcontents;
      back.fcontents = sides[i].fcontents;
      split_side(&sides[i], splitplane, &front, &back);
      assert(frontcount < MAX_BRUSHES_AT_LEVEL);
      assert(backcount < MAX_BRUSHES_AT_LEVEL);
      if (WINDING_VALID(front.win))
        fronts[frontcount++] = front;
      if (WINDING_VALID(back.win))
        backs[backcount++] = back;
      continue;

      // fronts[frontcount++] =  sides[i];
      // backs[backcount++] = sides[i];
      continue;
    }
    assert(frontcount < MAX_BRUSHES_AT_LEVEL);
    assert(backcount < MAX_BRUSHES_AT_LEVEL);
    if (result & PSIDE_FRONT)
      fronts[frontcount++] = sides[i];
    if (result & PSIDE_BACK)
      backs[backcount++] = sides[i];
  }

  if (frontcount == 0 || backcount == 0)
  {
    // All sides lie on one side (or are coplanar) � make a leaf
    int idx = tree->leaf_count++;
    tree->leaves[idx].face_start = -1;
    tree->leaves[idx].face_count = 0;
    // Use the actual contents of the side(s), not just EMPTY
    tree->leaves[idx].contents = (sidecount > 0) ? sides[0].bcontents : CONTENTS_EMPTY;
    if (tree->leaves[idx].contents == CONTENTS_EMPTY)
      gLeafEmpty++;
    else
      gLeafSolid++;
    free(fronts);
    free(backs);
    return LEAF(idx);
  }

  // printf("DEPTH %d", depth); print_side_array_debug(fr, sidecount);
  //  recurse
  int node_idx = tree->node_count++;
  tree->nodes[node_idx].plane = gBSPplanes[splitplane];
  tree->nodes[node_idx].front = BSP_BuildTree_r(tree, fronts, frontcount, depth + 1);
  tree->nodes[node_idx].back = BSP_BuildTree_r(tree, backs, backcount, depth + 1);

  free(fronts);
  free(backs);
  return node_idx;
}

static void BSP_CSGPass(bsp_tree_t *tree)
{
  for (int i = 0; i < gEditorBrushArray->count; i++)
  {
    brush_t *brush = &gEditorBrushArray->brushes[i];
    if (brush->is_entity)
      continue; // skip skybox etc.

    for (int s = 0; s < brush->side_count; s++)
    {
      brush_side_t *side = &brush->sides[s];
      winding_t face = build_brush_face(brush, s); // your existing function
      if (WINDING_DEGEN(face))
        continue;

      face_t f = {0};
      f.win = face;
      f.plane = side->plane; // Quake backface test uses this
      f.contents_front = CONTENTS_EMPTY;
      f.contents_back = brush->contents;
      f.material_id = side->material_id;
      ComputePlanarUVs(&f.win, f.plane, f.uvs, 0.01f);

      Vector u_axis, v_axis;
      Vector up = {0, 1, 0};
      if (fabsf(f.plane.normal.y) > 0.99f)
        up = (Vector){0, 0, 1};
      u_axis = VectorCrossNormalise(up, f.plane.normal);
      v_axis = VectorCrossNormalise(f.plane.normal, u_axis);

      f.tangent = u_axis;
      f.bit_tangent = v_axis;

      Vector col;
      Vector n = VectorInit(
          fabsf(f.plane.normal.x),
          fabsf(f.plane.normal.y),
          fabsf(f.plane.normal.z));
      if (n.x > n.y && n.x > n.z)
        col = (side->plane.normal.x > 0) ? axis_colours[0] : axis_colours[3];
      else if (n.y > n.z)
        col = (side->plane.normal.y > 0) ? axis_colours[1] : axis_colours[4];
      else
        col = (side->plane.normal.z > 0) ? axis_colours[2] : axis_colours[5];
      f.debug_colour = col;
      add_face_to_tree(tree, f);
    }
  }
}

// Recursively walk the tree, clip a bounding box by node planes,
// and collect the centre of each leaf's box.
static void BSP_CollectLeafPoints(bsp_tree_t *tree, int nodeIdx,
                                  Vector mins, Vector maxs,
                                  Vector *points, int *count)
{
  if (IS_LEAF(nodeIdx))
  {
    int leaf = LEAF_IDX(nodeIdx);
    points[leaf] = VectorScale(VectorAdd(mins, maxs), 0.5f);
    (*count)++;
    return;
  }
  bsp_node_t *node = &tree->nodes[nodeIdx];
  plane_t p = node->plane;

  // Clip the box to front side (d >= 0) and back side (d < 0)
  // For axis-aligned planes (the vast majority), this is exact.
  Vector fMins = mins, fMaxs = maxs;
  Vector bMins = mins, bMaxs = maxs;

  if (fabsf(p.normal.x) > 0.99f)
  {
    if (p.normal.x > 0)
    {
      fMins.x = fmaxf(fMins.x, p.dist);
      bMaxs.x = fminf(bMaxs.x, p.dist);
    }
    else
    {
      fMaxs.x = fminf(fMaxs.x, -p.dist);
      bMins.x = fmaxf(bMins.x, -p.dist);
    }
  }
  else if (fabsf(p.normal.y) > 0.99f)
  {
    if (p.normal.y > 0)
    {
      fMins.y = fmaxf(fMins.y, p.dist);
      bMaxs.y = fminf(bMaxs.y, p.dist);
    }
    else
    {
      fMaxs.y = fminf(fMaxs.y, -p.dist);
      bMins.y = fmaxf(bMins.y, -p.dist);
    }
  }
  else if (fabsf(p.normal.z) > 0.99f)
  {
    if (p.normal.z > 0)
    {
      fMins.z = fmaxf(fMins.z, p.dist);
      bMaxs.z = fminf(bMaxs.z, p.dist);
    }
    else
    {
      fMaxs.z = fminf(fMaxs.z, -p.dist);
      bMins.z = fmaxf(bMins.z, -p.dist);
    }
  }
  // (For non-axis planes, just pass the same box – still works, less precise)

  BSP_CollectLeafPoints(tree, node->front, fMins, fMaxs, points, count);
  BSP_CollectLeafPoints(tree, node->back, bMins, bMaxs, points, count);
}

bsp_tree_t *BSP2_Compile(void)
{
  for (int i = 0; i < face_mesh_count; i++)
  {
    if (FACE_MESHES[i])
    {
      MeshDestroy(FACE_MESHES[i]);
    }
  }
  face_mesh_count = 0;

  bsp_tree_t *tree = malloc(sizeof(bsp_tree_t));
  assert(tree);
  bsp_container_init(tree);
  plane_array_init();
  side_array_fill();

  BSP_DEBUG_SPLITPLANES_COUNT = 0;

  int root = BSP_BuildTree_r(tree, gBSPsides, gBSPsidecount, 0);
  printf("ROOT=%d\n", root);
  


  // Allocate per-leaf point array
  Vector *leafPoints = malloc(sizeof(Vector) * tree->leaf_count);
  int numPoints = 0;

  // Start with a world bounding box (adjust to your map size)
  Vector worldMins = {-4096, -4096, -4096};
  Vector worldMaxs = { 4096,  4096,  4096};

  BSP_CollectLeafPoints(tree, 0, worldMins, worldMaxs, leafPoints, &numPoints);

  // Now classify each leaf
  for (int i = 0; i < tree->leaf_count; i++) {
      int contents = CONTENTS_EMPTY;
      // Test against all CSG brushes (non-entity)
      for (int b = 0; b < gEditorBrushArray->count; b++) {
          brush_t *brush = &gEditorBrushArray->brushes[b];
          if (brush->is_entity) continue;
          if (point_in_brush(brush, leafPoints[i])) {
              contents = brush->contents;
              break;
          }
      }
      tree->leaves[i].contents = contents;
  }
  free(leafPoints);
  int empty = 0, solid = 0;
  for (int i = 0; i < tree->leaf_count; i++) {
      if (tree->leaves[i].contents == CONTENTS_EMPTY) empty++;
      else solid++;
  }
  printf("[BSP] After classification: %d empty, %d solid\n", empty, solid);

  BSP_CSGPass(tree);
  printf("[BSP] Building tree from %zu brushes\n", gEditorBrushArray->count);

  printf("[BSP] %zu nodes, %zu leaves\n", tree->node_count, tree->leaf_count);

  printf("[BSP]: %zu debug plane(s)\n", BSP_DEBUG_SPLITPLANES_COUNT);
  for (int i = 0; i < BSP_DEBUG_SPLITPLANES_COUNT; i++)
  {
    BSP_DEBUG_SPLITPLANES_MESHES[i] = MeshFromPlane(BSP_DEBUG_SPLITPLANES[i]);
    MeshUpload(BSP_DEBUG_SPLITPLANES_MESHES[i], GL_STATIC_DRAW);
  }
  /*
  printf("[BSP] Leaf contents:\n");
  for (int i = 0; i < tree->leaf_count; i++)
  {
    printf("  Leaf %d: contents=%d (%s)\n", i, tree->leaves[i].contents,
           tree->leaves[i].contents == CONTENTS_EMPTY ? "EMPTY" : tree->leaves[i].contents == CONTENTS_SOLID ? "SOLID"
                                                                                                             : "OTHER");
  }
                                                                                                             */
  mesh_array_fill(tree);
  return tree;
}

// REWRITE ENDS

// OLD IGNORE THIS

bsp_tree_t *BSP_Compile(void)
{
  for (int i = 0; i < face_mesh_count; i++)
  {
    if (FACE_MESHES[i])
    {
      MeshDestroy(FACE_MESHES[i]);
    }
  }
  face_mesh_count = 0;

  bsp_tree_t *tree = malloc(sizeof(bsp_tree_t));
  assert(tree);
  bsp_container_init(tree);

  brush_t **brush_list = malloc(sizeof(brush_t *) * MAX_BRUSHES);
  int brush_count = 0;
  BSP_DEBUG_SPLITPLANES_COUNT = 0;

  for (int i = 0; i < gEditorBrushArray->count; i++)
  {
    brush_t *src = &gEditorBrushArray->brushes[i];
    if (src->is_entity)
      continue;
    brush_t *copy = malloc(sizeof(brush_t));
    memcpy(copy, src, sizeof(brush_t));
    brush_list[brush_count++] = copy;
  }

  printf("[BSP] Building tree from %d brushes\n", brush_count);
  build_tree(brush_list, brush_count, tree, 0);
  BSP_CSGPass(tree);
  printf("[BSP] %zu nodes, %zu leaves\n", tree->node_count, tree->leaf_count);

  for (int i = 0; i < brush_count; i++)
    free(brush_list[i]);

  printf("[BSP]: %zu debug plane(s)\n", BSP_DEBUG_SPLITPLANES_COUNT);
  for (int i = 0; i < BSP_DEBUG_SPLITPLANES_COUNT; i++)
  {
    BSP_DEBUG_SPLITPLANES_MESHES[i] = MeshFromPlane(BSP_DEBUG_SPLITPLANES[i]);
    MeshUpload(BSP_DEBUG_SPLITPLANES_MESHES[i], GL_STATIC_DRAW);
  }
  /*
  printf("[BSP] Leaf contents:\n");
  for (int i = 0; i < tree->leaf_count; i++)
  {
    printf("  Leaf %d: contents=%d (%s)\n", i, tree->leaves[i].contents,
           tree->leaves[i].contents == CONTENTS_EMPTY ? "EMPTY" : tree->leaves[i].contents == CONTENTS_SOLID ? "SOLID"
                                                                                                             : "OTHER");
  }
                                                                                                             */
  mesh_array_fill(tree);

  return tree;
}
