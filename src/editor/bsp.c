#include "editor/bsp.h"

#define ON_EPSILON 0.01f

#ifndef MAT_NODRAW
#define MAT_NODRAW -1
#endif

// Negative index means leaf: -(leaf_index + 1)
#define LEAF_INDEX(leaf_idx) (-((leaf_idx) + 1))
#define NODE_FROM_LEAF(neg) (-(neg) - 1)

#define MAX_SPLIT_CANDIDATES 512
#define INITIAL_CAPACITY 128
#define MAX_BRUSH_STACK 4096  // Max brushes at any recursion level

typedef struct
{
  plane_t plane;
  int front_count;
  int back_count;
  int crossing_count;
  float score;
} split_candidate_t;


// Returns -1 = behind, 0 = on, 1 = front
int PointOnPlaneSide(Vector point, plane_t plane)
{
  float d = VectorDot(point, plane.normal) - plane.dist;
  if (d > ON_EPSILON)
    return 1;
  if (d < -ON_EPSILON)
    return -1;
  return 0;
}

// Result flags
#define BRUSH_SPLIT_FRONT 1
#define BRUSH_SPLIT_BACK 2
#define BRUSH_SPLIT_CROSSING (BRUSH_SPLIT_FRONT | BRUSH_SPLIT_BACK)

// Double capacity, returns 1 on success
static int EnsureNodeCapacity(bsp_container_t *tree)
{
  if (tree->node_count >= tree->node_capacity)
  {
    size_t newcap = tree->node_capacity * 2;
    bsp_node_t *new_nodes = realloc(tree->nodes, sizeof(bsp_node_t) * newcap);
    if (!new_nodes) return 0;
    tree->nodes = new_nodes;
    tree->node_capacity = newcap;
  }
  return 1;
}

static int EnsureLeafCapacity(bsp_container_t *tree)
{
  if (tree->leaf_count >= tree->leaf_capacity)
  {
    size_t newcap = tree->leaf_capacity * 2;
    bsp_leaf_t *new_leaves = realloc(tree->leaves, sizeof(bsp_leaf_t) * newcap);
    if (!new_leaves) return 0;
    tree->leaves = new_leaves;
    tree->leaf_capacity = newcap;
  }
  return 1;
}

int Brush_Split(brush_t *in, plane_t splitPlane, brush_t **outFront, brush_t **outBack)
{
  if (in->is_entity)
  {
    printf("[BSP]: Entity brush attempted - CHECK Brush_Split()-bsp.c, aborting...");
    exit(1);
  }

  *outFront = NULL;
  *outBack = NULL;

  brush_t *f = calloc(1, sizeof(brush_t));
  brush_t *b = calloc(1, sizeof(brush_t));
  if (!f || !b)
  {
    free(f);
    free(b);
    return 0;
  }

  f->contents = in->contents;
  b->contents = in->contents;
  f->is_entity = in->is_entity;
  b->is_entity = in->is_entity;

  for (size_t i = 0; i < in->side_count; i++)
  {
    brush_side_t *side = &in->sides[i];
    plane_t plane = side->plane;

    winding_t wface = base_winding(plane);
    for (size_t j = 0; j < in->side_count; j++)
    {
      if (i == j) continue;
      plane_t cplane = PlaneReverse(in->sides[j].plane);
      wface = clip_winding(&wface, cplane);
      if (wface.count < 3) break;
    }
    if (wface.count < 3) continue;

    winding_t wfront = clip_winding(&wface, PlaneReverse(splitPlane));
    winding_t wback = clip_winding(&wface, splitPlane);

    if (wfront.count >= 3 && f->side_count < MAX_BRUSH_FACES)
    {
      brush_side_t *newside = &f->sides[f->side_count++];
      *newside = *side;
      newside->plane = plane;
    }
    if (wback.count >= 3 && b->side_count < MAX_BRUSH_FACES)
    {
      brush_side_t *newside = &b->sides[b->side_count++];
      *newside = *side;
      newside->plane = plane;
    }
  }

  if (f->side_count > 0 && f->side_count < MAX_BRUSH_FACES)
  {
    brush_side_t *seal = &f->sides[f->side_count++];
    memset(seal, 0, sizeof(brush_side_t));
    seal->plane = splitPlane;
    seal->material_id = MAT_NODRAW;
  }
  if (b->side_count > 0 && b->side_count < MAX_BRUSH_FACES)
  {
    brush_side_t *seal = &b->sides[b->side_count++];
    memset(seal, 0, sizeof(brush_side_t));
    seal->plane = PlaneReverse(splitPlane);
    seal->material_id = MAT_NODRAW;
  }

  int result = 0;
  if (f->side_count >= 4)
  {
    *outFront = f;
    result |= BRUSH_SPLIT_FRONT;
  }
  else free(f);

  if (b->side_count >= 4)
  {
    *outBack = b;
    result |= BRUSH_SPLIT_BACK;
  }
  else free(b);

  return result;
}

plane_t ChooseBestSplitPlane(brush_t *brushes, int brushCount)
{
  split_candidate_t candidates[MAX_SPLIT_CANDIDATES];
  int candCount = 0;

  for (int i = 0; i < brushCount; i++)
  {
    brush_t *b = &brushes[i];
    if (b->is_entity) continue;
    for (int j = 0; j < b->side_count; j++)
    {
      plane_t p = b->sides[j].plane;
      int found = 0;
      for (int k = 0; k < candCount; k++)
      {
        if (VectorEqual(candidates[k].plane.normal, p.normal) &&
            fabs(candidates[k].plane.dist - p.dist) < 0.01f)
        {
          found = 1;
          break;
        }
      }
      if (!found && candCount < MAX_SPLIT_CANDIDATES)
      {
        candidates[candCount].plane = p;
        candidates[candCount].front_count = 0;
        candidates[candCount].back_count = 0;
        candidates[candCount].crossing_count = 0;
        candCount++;
      }
    }
  }

  // If no candidates, return a default plane (shouldn't happen with valid brushes)
  if (candCount == 0)
  {
    plane_t def = {0};
    return def;
  }

  for (int c = 0; c < candCount; c++)
  {
    plane_t test = candidates[c].plane;
    int front = 0, back = 0, cross = 0;
    for (int i = 0; i < brushCount; i++)
    {
      brush_t *b = &brushes[i];
      if (b->is_entity) continue;
      brush_t *f = NULL, *bk = NULL;
      int res = Brush_Split(b, test, &f, &bk);
      if (res == BRUSH_SPLIT_CROSSING) cross++;
      else if (res & BRUSH_SPLIT_FRONT) front++;
      else if (res & BRUSH_SPLIT_BACK) back++;
      if (f) free(f);
      if (bk) free(bk);
    }
    candidates[c].front_count = front;
    candidates[c].back_count = back;
    candidates[c].crossing_count = cross;

    float balance = 1.0f - fabs((float)(front - back) / (float)(front + back + 1));
    candidates[c].score = balance - (cross * 2.0f);
  }

  int best = 0;
  for (int i = 1; i < candCount; i++)
  {
    if (candidates[i].score > candidates[best].score)
      best = i;
  }
  return candidates[best].plane;
}

#define MAX_RECURSION_DEPTH 64

int BuildBSP_r(brush_t *brushes, int brushCount, bsp_container_t *tree, int depth)
{
  // Safety: depth limit
  if (depth > MAX_RECURSION_DEPTH)
  {
    printf("[BSP]: Max recursion depth reached, forcing leaf\n");
    if (!EnsureLeafCapacity(tree)) return LEAF_INDEX(0);
    
    bsp_leaf_t leaf;
    leaf.contents = (brushCount > 0) ? brushes[0].contents : CONTENTS_EMPTY;
    int leafIdx = tree->leaf_count++;
    tree->leaves[leafIdx] = leaf;
    return LEAF_INDEX(leafIdx);
  }

  // No brushes -> empty leaf
  if (brushCount == 0)
  {
    if (!EnsureLeafCapacity(tree)) return LEAF_INDEX(0);
    
    bsp_leaf_t leaf;
    leaf.contents = CONTENTS_EMPTY;
    int leafIdx = tree->leaf_count++;
    tree->leaves[leafIdx] = leaf;
    return LEAF_INDEX(leafIdx);
  }

  // Check if all brushes have the same contents
  int homogeneous = 1;
  int commonContents = brushes[0].contents;
  for (int i = 1; i < brushCount; i++)
  {
    if (brushes[i].contents != commonContents)
    {
      homogeneous = 0;
      break;
    }
  }
  if (homogeneous)
  {
    if (!EnsureLeafCapacity(tree)) return LEAF_INDEX(0);
    
    bsp_leaf_t leaf;
    leaf.contents = commonContents;
    int leafIdx = tree->leaf_count++;
    tree->leaves[leafIdx] = leaf;
    return LEAF_INDEX(leafIdx);
  }

  // Choose splitting plane
  plane_t splitPlane = ChooseBestSplitPlane(brushes, brushCount);

  // Allocate temporary arrays on heap to avoid stack overflow
  brush_t **frontBrushes = malloc(sizeof(brush_t*) * MAX_BRUSH_STACK);
  brush_t **backBrushes = malloc(sizeof(brush_t*) * MAX_BRUSH_STACK);
  if (!frontBrushes || !backBrushes)
  {
    printf("[BSP]: Failed to allocate split arrays\n");
    free(frontBrushes);
    free(backBrushes);
    
    if (!EnsureLeafCapacity(tree)) return LEAF_INDEX(0);
    bsp_leaf_t leaf;
    leaf.contents = commonContents;
    int leafIdx = tree->leaf_count++;
    tree->leaves[leafIdx] = leaf;
    return LEAF_INDEX(leafIdx);
  }

  int frontCount = 0, backCount = 0;

  for (int i = 0; i < brushCount; i++)
  {
    brush_t *f = NULL, *b = NULL;
    int res = Brush_Split(&brushes[i], splitPlane, &f, &b);
    
    if (res & BRUSH_SPLIT_FRONT)
    {
      if (frontCount < MAX_BRUSH_STACK)
        frontBrushes[frontCount++] = f;
      else
        free(f);
    }
    if (res & BRUSH_SPLIT_BACK)
    {
      if (backCount < MAX_BRUSH_STACK)
        backBrushes[backCount++] = b;
      else
        free(b);
    }
  }

  // Ensure node capacity before creating node
  if (!EnsureNodeCapacity(tree))
  {
    free(frontBrushes);
    free(backBrushes);
    return LEAF_INDEX(0);
  }

  int nodeIdx = tree->node_count++;
  bsp_node_t *node = &tree->nodes[nodeIdx];
  node->plane = splitPlane;

  // CRITICAL FIX: use backCount, not frontCount
  node->front = BuildBSP_r(*frontBrushes, frontCount, tree, depth + 1);
  node->back  = BuildBSP_r(*backBrushes, backCount, tree, depth + 1);

  // Free the temporary split brushes
  for (int i = 0; i < frontCount; i++) free(frontBrushes[i]);
  for (int i = 0; i < backCount; i++) free(backBrushes[i]);
  free(frontBrushes);
  free(backBrushes);

  return nodeIdx;
}

void BSPStart(void)
{
  bsp_container_t tree;
  
  tree.nodes = malloc(INITIAL_CAPACITY * sizeof(bsp_node_t));
  tree.node_count = 0;
  tree.node_capacity = INITIAL_CAPACITY;
  
  tree.leaves = malloc(INITIAL_CAPACITY * sizeof(bsp_leaf_t));
  tree.leaf_count = 0;
  tree.leaf_capacity = INITIAL_CAPACITY;
  
  tree.faces = NULL;
  tree.face_count = 0;

  if (!tree.nodes || !tree.leaves)
  {
    printf("[BSP]: Failed to allocate initial arrays\n");
    free(tree.nodes);
    free(tree.leaves);
    return;
  }

  // Filter out entity brushes
  brush_t *structural = malloc(sizeof(brush_t) * MAX_BRUSHES);
  if (!structural)
  {
    printf("[BSP]: Failed to allocate structural brush array\n");
    free(tree.nodes);
    free(tree.leaves);
    return;
  }

  int count = 0;
  for (int i = 0; i < gEditorBrushArray->count; i++)
  {
    brush_t *b = &gEditorBrushArray->brushes[i];
    if (!b->is_entity)
    {
      structural[count++] = *b;
    }
  }

  printf("[BSP]: Starting BSP build with %d structural brushes\n", count);
  
  int root = BuildBSP_r(structural, count, &tree, 0);
  
  printf("[BSP]: BSP built - %d nodes, %d leaves\n", tree.node_count, tree.leaf_count);
  printf("[BSP]: Root node index: %d\n", root);

  // TODO: Save BSP to file or store globally for later use
  // For now, just clean up
  free(structural);
  
  // In production, you'd keep tree around. For testing, clean up:
  free(tree.nodes);
  free(tree.leaves);
}