#include "editor/bsp.h"
#include "types/types_vector.h"
#include <assert.h>
#include <float.h>

// This must be the 5th rewrite of this file - 08/06/2026 03:03,
// trying to keep it tidy to stop me from chimping out

// #define BSP_CONT_CAPACITY_NODE_INIT 64
// #define BSP_CONT_CAPACITY_LEAF_INIT 64
// #define BSP_CONT_CAPACITY_FACE_INIT 128

#define MAX_BSP_NODES 1024
#define MAX_BSP_LEAVES 1024
#define MAX_TREE_DEPTH 16
#define MAX_BRUSHES_AT_LEVEL 4096

#define MAX_CANDIDATES (MAX_BRUSH_FACES)
#define LEAF(i) (-((i) + 1))
#define LEAF_IDX(v) (-(v) - 1)
#define IS_LEAF(v) ((v) < 0)

#define BSP_CANDIDATE_STRADDLE_PENALTY 2.0F
#define BSP_CANDIDATE_AXIAL_BONUS 0.5F

// Temporary debug purposes

plane_t BSP_DEBUG_SPLITPLANES[4096];
mesh_t *BSP_DEBUG_SPLITPLANES_MESHES[4096];
int split_plane_count = 0;

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
  cont->face_capacity = MAX_BRUSH_FACES;

  cont->nodes = malloc(sizeof(bsp_node_t) * cont->node_capacity);
  cont->leaves = malloc(sizeof(bsp_leaf_t) * cont->leaf_capacity);
  cont->faces = malloc(sizeof(bsp_face_t) * cont->face_capacity);

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
  return face;
}

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
    printf("  plane: normal(%.1f, %.1f, %.1f) dist=%.1f -> score=%.2f (f=%d b=%d x=%d)\n",
           candidates[c].plane.normal.x,
           candidates[c].plane.normal.y,
           candidates[c].plane.normal.z,
           candidates[c].plane.dist,
           candidates[c].score,
           front_count,
           back_count,
           straddle_count);
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
  if (!check_plane_dupe(BSP_DEBUG_SPLITPLANES, split_plane_count, split))
    BSP_DEBUG_SPLITPLANES[split_plane_count++] = split;

  // Split brushes
  brush_t *fronts[MAX_BRUSHES_AT_LEVEL];
  brush_t *backs[MAX_BRUSHES_AT_LEVEL];
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
    for (int i = 0; i < front_count; i++)
      free(fronts[i]);
    for (int i = 0; i < back_count; i++)
      free(backs[i]);

    // Determine leaf contents by sampling the first brush's center
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

bsp_tree_t *BSP_Compile(void)
{
  bsp_tree_t *tree = malloc(sizeof(bsp_tree_t));
  assert(tree);
  bsp_container_init(tree);

  brush_t *brush_list[MAX_BRUSHES];
  int brush_count = 0;
  split_plane_count = 0;

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
  printf("[BSP] %d nodes, %d leaves\n", tree->node_count, tree->leaf_count);

  for (int i = 0; i < brush_count; i++)
    free(brush_list[i]);

  for (int i = 0; i < split_plane_count; i++)
  {
    BSP_DEBUG_SPLITPLANES_MESHES[i] = MeshFromPlane(BSP_DEBUG_SPLITPLANES[i]);
    MeshUpload(BSP_DEBUG_SPLITPLANES_MESHES[i], GL_STATIC_DRAW);
  }

  return tree;
}