#include "editor/bsp.h"
#include "types/types_vector.h"
#include <assert.h>
#include <float.h>
#include <stdint.h>
#include <stdlib.h>

// This must be the 5th rewrite of this file - 08/06/2026 03:03,
// trying to keep it tidy to stop me from chimping out

// #define BSP_CONT_CAPACITY_NODE_INIT 64
// #define BSP_CONT_CAPACITY_LEAF_INIT 64
// #define BSP_CONT_CAPACITY_FACE_INIT 128

#define MAX_BSP_NODES 1024
#define MAX_BSP_LEAVES 1024
#define MAX_TREE_DEPTH 32
#define MAX_BRUSHES_AT_LEVEL 4096

#define MAX_CANDIDATES (512)
#define LEAF(i) (-((i) + 1))
#define LEAF_IDX(v) (-(v) - 1)
#define IS_LEAF(v) ((v) < 0)

#define BSP_CANDIDATE_STRADDLE_PENALTY 2.0F
#define BSP_CANDIDATE_AXIAL_BONUS 0.5F

Vector axis_colours[6] = {
    {1, 0, 0},   // +X  red
    {0, 1, 0},   // +Y  green
    {0, 0, 1},   // +Z  blue
    {1, 0, 1},   // -X  magenta
    {1, 1, 0},   // -Y  yellow
    {0, 1, 1}    // -Z  cyan
};


// Temporary debug purposes

plane_t BSP_DEBUG_SPLITPLANES[4096];
mesh_t *BSP_DEBUG_SPLITPLANES_MESHES[4096];
size_t BSP_DEBUG_SPLITPLANES_COUNT = 0;

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

static void WindingEnsureOrientation(winding_t* w, Vector normal)
{
  if (w->count < 3)
    return;

  Vector e1 = VectorSub(w->v[1], w->v[0]);
  Vector e2 = VectorSub(w->v[2], w->v[0]);

  Vector geom = VectorCrossNormalise(e1, e2);

  if (VectorDot(geom, normal) < 0.0f)
  {
    for (int i = 0; i < w->count / 2; i++)
    {
      Vector tmp = w->v[i];
      w->v[i] = w->v[w->count - 1 - i];
      w->v[w->count - 1 - i] = tmp;
    }
  }
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
  brush_t **fronts = malloc(sizeof(brush_t*) * MAX_BRUSHES_AT_LEVEL);
  brush_t **backs  = malloc(sizeof(brush_t*) * MAX_BRUSHES_AT_LEVEL);
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
        split.dist
    );

    printf(
    "depth=%d brush_count=%d f=%d b=%d\n",
    depth,
    brush_count,
    front_count,
    back_count
);

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
static void FaceBounds(winding_t* w, Vector* mins, Vector* maxs)
{
  *mins = *maxs = w->v[0];
  for (int i = 1; i < w->count; i++) {
    if (w->v[i].x < mins->x) mins->x = w->v[i].x;
    if (w->v[i].y < mins->y) mins->y = w->v[i].y;
    if (w->v[i].z < mins->z) mins->z = w->v[i].z;
    if (w->v[i].x > maxs->x) maxs->x = w->v[i].x;
    if (w->v[i].y > maxs->y) maxs->y = w->v[i].y;
    if (w->v[i].z > maxs->z) maxs->z = w->v[i].z;
  }
}

static void ComputePlanarUVs(winding_t* w, plane_t plane, Vector2* uvs_out, float scale)
{
  // Build basis vectors on the plane
  Vector u_axis, v_axis;
  Vector up = { 0, 1, 0 };
  if (fabsf(plane.normal.y) > 0.99f) up = (Vector){ 0, 0, 1 };

  u_axis = VectorCrossNormalise(up, plane.normal);
  v_axis = VectorCrossNormalise(plane.normal, u_axis);

  for (int i = 0; i < w->count; i++) {
    uvs_out[i].x = VectorDot(w->v[i], u_axis) * scale;
    uvs_out[i].y = VectorDot(w->v[i], v_axis) * scale;
  }
}

static void BSP_CSGPass(bsp_tree_t* tree)
{
  for (int i = 0; i < gEditorBrushArray->count; i++) {
    brush_t* brush = &gEditorBrushArray->brushes[i];
    if (brush->is_entity) continue;          // skip skybox etc.

    for (int s = 0; s < brush->side_count; s++) {
      brush_side_t* side = &brush->sides[s];
      winding_t face = build_brush_face(brush, s);   // your existing function
      if (WINDING_DEGEN(face)) continue;

      face_t f = { 0 };
      f.win = face;
      f.plane = side->plane;                // Quake backface test uses this
      f.contents_front = CONTENTS_EMPTY;
      f.contents_back = brush->contents;
      f.material_id = side->material_id;
      ComputePlanarUVs(&f.win, f.plane, f.uvs, 0.01f);


      Vector u_axis, v_axis;
      Vector up = { 0, 1, 0 };
      if (fabsf(f.plane.normal.y) > 0.99f) up = (Vector){ 0, 0, 1 };
      u_axis = VectorCrossNormalise(up, f.plane.normal);
      v_axis = VectorCrossNormalise(f.plane.normal, u_axis);
      
      f.tangent = u_axis;
      f.bit_tangent = v_axis;



      Vector col;
      Vector n = VectorInit(
        fabsf(f.plane.normal.x),
        fabsf(f.plane.normal.y),
        fabsf(f.plane.normal.z)
      );
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


static void split_face(face_t *in, plane_t splane, face_t **front, face_t **back)
{
  if (!in || !front || !back)
    return;
  *front = NULL;
  *back = NULL;

  winding_t wfront = clip_winding(&in->win, PlaneReverse(splane));
  winding_t wback = clip_winding(&in->win, splane);

  if (WINDING_VALID(wfront))
  {
    *front = malloc(sizeof(face_t));
    **front = *in; // copy face
    (*front)->win = wfront;
  }
  if (WINDING_VALID(wback))
  {
    *back = malloc(sizeof(face_t));
    **back = *in;
    (*back)->win = wback;
  }
}

static int classify_face_side(face_t *face, plane_t splane)
{
  int front = 0, back = 0;
  // decides which side of a plane a particlar face belongs based on the position of its vertices
  for (int i = 0; i < face->win.count; i++)
  {
    float d = VectorDot(face->win.v[i], splane.normal) - splane.dist;
    if (d > EPSILON)
      front = 1;
    if (d < -EPSILON)
      back = 1;
    if (front && back)
      return SPLIT_BOTH;
  }
  if (front)
    return SPLIT_FRONT;
  if (back)
    return SPLIT_BACK;
  return SPLIT_FRONT;
}

static plane_t choose_splane_face(face_t **faces, int face_count)
{

  plane_t candidates[MAX_CANDIDATES];
  int cand_count = 0;
  // Improved method? - not storing score
  // gather candidates
  for (int i = 0; i < face_count; i++)
  {
    face_t *face = faces[i];
    // technically a face can contribute any plane from its winding - or brush face
    // choose the plane from first 3 vertices
    if (WINDING_DEGEN(face->win))
      continue;
    // plane creation
    Vector v0 = face->win.v[0];
    Vector v1 = face->win.v[1];
    Vector v2 = face->win.v[2];
    Vector e1 = VectorSub(v1, v0);
    Vector e2 = VectorSub(v2, v0);
    Vector norm = VectorCrossNormalise(e1, e2);
    if (VectorMag(norm) < 0.001f)
      continue; // skip degenerate faces
    float dist = VectorDot(norm, v0);

    // In choose_splane_face, after computing the plane:
    plane_t plane = {.dist = dist, .normal = norm};
    plane_t plane_rev = {.dist = -dist, .normal = VectorNegate(norm)};

    // Add both
    if (!check_plane_dupe(candidates, cand_count, plane) && cand_count < MAX_CANDIDATES)
      candidates[cand_count++] = plane;
    if (!check_plane_dupe(candidates, cand_count, plane_rev) && cand_count < MAX_CANDIDATES)
      candidates[cand_count++] = plane_rev;
  }

  // fallback
  if (cand_count == 0)
  {
    return (plane_t){.dist = 0, .normal = VECTOR_AXIS_Y};
  }
  // Score the candidates and choose the best
  int best = 0;
  float best_score = -FLT_MAX;
  for (int c = 0; c < cand_count; c++)
  {
    int front_count = 0, back_count = 0, cross_count = 0;
    for (int i = 0; i < face_count; i++)
    {
      int result = classify_face_side(faces[i], candidates[c]);
      if (result == SPLIT_BOTH)
        cross_count++;
      else if (result == SPLIT_FRONT)
        front_count++;
      else if (result == SPLIT_BACK)
        back_count++;
      else
        back_count++;
    }
    printf("  cand %d: normal(%.1f,%.1f,%.1f) dist=%.1f -> f=%d b=%d x=%d %s\n",
           c, candidates[c].normal.x, candidates[c].normal.y, candidates[c].normal.z,
           candidates[c].dist, front_count, back_count, cross_count,
           (front_count == 0 || back_count == 0) ? "REJECTED" : "");
    if (front_count == 0 || back_count == 0)
      continue;
    float balance = 1.0f - fabsf((float)(front_count - back_count) / (float)(front_count + back_count + 1));
    float score = balance * 10.0f - cross_count * BSP_CANDIDATE_STRADDLE_PENALTY;
    if (score > best_score)
    {
      best_score = score;
      best = c;
    }
  }
  return candidates[best];
}

static int BuildFaceTree(face_t **faces, int face_count, bsp_tree_t *tree, int depth)
{
  // Base cases
  if (depth >= MAX_TREE_DEPTH || tree->node_count >= MAX_BSP_NODES)
  {
    // Make a leaf - determine contents from faces
    // int contents = CONTENTS_SOLID; // default solid
    // In BuildFaceTree, for leaf with faces:
    int contents = face_count > 0 ? faces[0]->contents_front : CONTENTS_SOLID;

    int idx = tree->leaf_count++;
    tree->leaves[idx].contents = contents;
    tree->leaves[idx].face_start = tree->face_count;
    tree->leaves[idx].face_count = 0;
    return LEAF(idx);
  }

  if (face_count == 0)
  {
    int idx = tree->leaf_count++;
    tree->leaves[idx].contents = CONTENTS_SOLID;
    tree->leaves[idx].face_start = -1;
    tree->leaves[idx].face_count = 0;
    return LEAF(idx);
  }

  /*
  // Check if all faces have same front contents -> can make a leaf
  int all_same = 1;
  for (int i = 1; i < face_count; i++)
  {
    if (faces[i]->contents_front != faces[0]->contents_front)
    {
      all_same = 0;
      break;
    }
  }

  if (all_same)
  {
    int idx = tree->leaf_count++;
    tree->leaves[idx].contents = faces[0]->contents_front;
    tree->leaves[idx].face_start = -1;
    tree->leaves[idx].face_count = 0;
    return LEAF(idx);
  }
  */

  // Pick split plane
  plane_t split = choose_splane_face(faces, face_count);
  if (!check_plane_dupe(BSP_DEBUG_SPLITPLANES, BSP_DEBUG_SPLITPLANES_COUNT, split))
  {
    BSP_DEBUG_SPLITPLANES[BSP_DEBUG_SPLITPLANES_COUNT++] = split;
  }

  // Split all faces
  // face_t *front_faces[256];
  // face_t *back_faces[256];
  face_t **front_faces = malloc(sizeof(face_t *) * 512);
  face_t **back_faces = malloc(sizeof(face_t *) * 512);
  int front_count = 0, back_count = 0;

  for (int i = 0; i < face_count; i++)
  {
    int side = classify_face_side(faces[i], split);

    if (side == SPLIT_FRONT)
    {
      front_faces[front_count++] = faces[i];
    }
    else if (side == SPLIT_BACK)
    {
      back_faces[back_count++] = faces[i];
    }
    else
    {
      // Split the face
      face_t *ff = NULL, *bf = NULL;
      split_face(faces[i], split, &ff, &bf);

      if (ff)
        front_faces[front_count++] = ff;
      if (bf)
        back_faces[back_count++] = bf;

      // Free the original since it was split
      //free(faces[i]);
    }
  }

  // If split didn't separate, force leaf
  if (front_count == 0 || back_count == 0)
  {
    // Free the faces allocated
    // for (int i = 0; i < front_count; i++) free(front_faces[i]);
    // for (int i = 0; i < back_count; i++) free(back_faces[i]);
    free(front_faces);
    free(back_faces);
    int contents = face_count > 0 ? faces[0]->contents_front : CONTENTS_SOLID;
    int idx = tree->leaf_count++;
    tree->leaves[idx].contents = contents;
    tree->leaves[idx].face_start = -1;
    tree->leaves[idx].face_count = 0;
    return LEAF(idx);
  }

  // Create node
  int node_idx = tree->node_count++;
  tree->nodes[node_idx].plane = split;
  tree->nodes[node_idx].front = BuildFaceTree(front_faces, front_count, tree, depth + 1);
  tree->nodes[node_idx].back = BuildFaceTree(back_faces, back_count, tree, depth + 1);

  free(front_faces);
  free(back_faces);

  return node_idx;
}

#include "rendering/render_commands.h"
#include "rendering/mesh.h"
// very inefficient and rough but it works for debugging
mesh_t *FACE_MESHES[10240];
size_t face_mesh_count = 0;

static mesh_t *face_to_mesh(face_t *face)
{
  if (!face) return NULL;
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
        .tangent = face->tangent
      };
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
    //printf("Face mesh added\n");
  }
}

void R_DrawBSPFaces(camera_t* camera, face_t *faces, int face_count)
{
  for (int i = 0; i < face_mesh_count; i++)
  {
    float dot = VectorDot(camera->pos, faces[i].plane.normal) - faces[i].plane.dist;
    if (dot < 0) {
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

void R_DrawBSPPlanes(){
  for (int i = 0; i < BSP_DEBUG_SPLITPLANES_COUNT; i++) {
  struct rcmd_t* rcmd = MEM_ARENA_ALLOC(gMemArena, sizeof(struct rcmd_t), alignof(struct rcmd_t));
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

bsp_tree_t *BSP_Compile(void)
{
  for (int i = 0; i < face_mesh_count; i++) {
    if (FACE_MESHES[i]) {
      MeshDestroy(FACE_MESHES[i]);
    }
  }
  face_mesh_count = 0;
  
  bsp_tree_t *tree = malloc(sizeof(bsp_tree_t));
  assert(tree);
  bsp_container_init(tree);

  brush_t** brush_list = malloc(sizeof(brush_t*) * MAX_BRUSHES);
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
  printf("[BSP] %d nodes, %d leaves\n", tree->node_count, tree->leaf_count);

  for (int i = 0; i < brush_count; i++)
    free(brush_list[i]);

  printf("[BSP]: %d debug plane(s)\n", BSP_DEBUG_SPLITPLANES_COUNT);
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
