#include "editor/csg.h"

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

static int brush_add_side(brush_t *in, plane_t plane, int material)
{
  if (in->side_count >= MAX_BRUSH_FACES)
    return 0;
  brush_side_t newside = {0};
  newside.plane = plane;
  newside.material_id = material;
  BrushSide_DefaultUVs(&newside);
  in->sides[in->side_count++] = newside;
  return 1;
}

// Splits a brush by a plane and returns two more, leaving the original unchanged
void CSG_SplitBrush(brush_t *in, const plane_t *plane, brush_t **front, brush_t **back)
{
  *front = *back = NULL;

  // --- Build all side windings (local space) ---
  winding_t side_winds[MAX_BRUSH_FACES];
  for (int i = 0; i < in->side_count; i++)
    side_winds[i] = build_brush_face(in, i);

  // --- Classify brush relative to local split plane ---
  int bits = 0;
  for (int i = 0; i < in->side_count; i++)
  {
    winding_t *w = &side_winds[i];
    for (int j = 0; j < w->count; j++)
    {
      float d = VectorDot(plane->normal, w->v[j]) - plane->dist;
      if (d > EPSILON)
        bits |= 1;
      if (d < -EPSILON)
        bits |= 2;
    }
  }

  // --- Entirely on one side? ---
  if (!(bits & 2))
  { // all front
    *front = malloc(sizeof(brush_t));
    memcpy(*front, in, sizeof(brush_t));
    return;
  }
  if (!(bits & 1))
  { // all back
    *back = malloc(sizeof(brush_t));
    memcpy(*back, in, sizeof(brush_t));
    return;
  }

  // --- Straddle – allocate two new brushes ---
  brush_t f = {0}, b = {0};
  f.pos = in->pos;
  f.contents = in->contents;
  b.pos = in->pos;
  b.contents = in->contents;

  // --- Clip each side winding ---
  for (int i = 0; i < in->side_count; i++)
  {
    winding_t *w = &side_winds[i];
    if (w->count < 3)
      continue;

    winding_t wfront = clip_winding(w, PlaneReverse(*plane));
    winding_t wback = clip_winding(w, *plane);

    // The original side’s plane is already local – copy it directly
    if (WINDING_VALID(wfront) && f.side_count < MAX_BRUSH_FACES)
    {
      f.sides[f.side_count] = in->sides[i]; // local plane, no conversion
      f.side_count++;
    }
    if (WINDING_VALID(wback) && b.side_count < MAX_BRUSH_FACES)
    {
      b.sides[b.side_count] = in->sides[i];
      b.side_count++;
    }
  }

  // --- Seal the cut: add the split plane (world‑space) via brush_add_side ---
  // The front brush gets the reversed plane, back gets the normal plane.
  brush_add_side(&f, PlaneReverse(*plane), 0);
  brush_add_side(&b, *plane, 0);

  // --- Validate and copy out ---
  if (f.side_count >= 4)
  {
    *front = malloc(sizeof(brush_t));
    memcpy(*front, &f, sizeof(brush_t));
  }
  if (b.side_count >= 4)
  {
    *back = malloc(sizeof(brush_t));
    memcpy(*back, &b, sizeof(brush_t));
  }
}

// Subtract the volume of `subtract` from `src`.
// Both brushes must be convex and have outward‑pointing normals.
// Returns an array of convex brushes (src \ subtract) and the count.
int CSG_SubtractBrush(const brush_t *src, const brush_t *subtract,
                        brush_t **out_pieces)
{
  // Start with src as the only "in" piece
  brush_t **result = NULL;
  int resultCount = 0;

  brush_t *in = malloc(sizeof(brush_t));
  *in = *src;      // working brush – will be cut down
  int inValid = 1; // flag whether `in` is still valid

  for (int i = 0; i < subtract->side_count && inValid; i++)
  {

    plane_t world = subtract->sides[i].plane;

    brush_t *front = NULL, *back = NULL;
    CSG_SplitBrush(in, &world, &front, &back);

    // Front piece → outside this plane → add to result list
    if (front)
    {
      result = realloc(result, sizeof(brush_t *) * (resultCount + 1));
      result[resultCount++] = front;
    }

    // Back piece → still inside → continues as the new `in`
    if (back)
    {
      free(in);
      in = back; // `in` now points to the back piece
      inValid = 1;
    }
    else
    {
      // No back piece → nothing left to cut
      free(in);
      in = NULL;
      inValid = 0;
    }
  }

  // If something remains in `in`, it's fully inside the subtractor → discard
  if (inValid && in)
  {
    free(in);
  }

  // Copy result pointers into a flat array of brush_t
  *out_pieces = malloc(sizeof(brush_t) * resultCount);
  for (int i = 0; i < resultCount; i++)
  {
    (*out_pieces)[i] = *result[i];
    free(result[i]);
  }
  free(result);
  return resultCount;
}


/*
void EditorCreate_BrushRoom_CSG(editor_brush_array *arr, Vector mins, Vector maxs,
                                int material, int is_entity)
{
  float t = 16.0f; // wall thickness
  brush_t outer = make_brush_cube(mins, maxs);
  brush_t inner = make_brush_cube(
      (Vector){mins.x + t, mins.y + t, mins.z + t},
      (Vector){maxs.x - t, maxs.y - t, maxs.z - t});

  brush_t *pieces;
  int n = CSG_Subtract(&outer, &inner, &pieces);

  for (int i = 0; i < n; i++)
  {
    pieces[i].is_entity = is_entity;
    pieces[i].contents = CONTENTS_SOLID;
    for (int s = 0; s < pieces[i].side_count; s++)
      pieces[i].sides[s].material_id = material;
    arr->brushes[arr->count++] = pieces[i];
  }
  free(pieces);
  printf("[EDITOR]: Hollow room via CSG, %d piece(s)\n", n);
}
*/


/*
void EditorCreate_BrushRoom_CSG(editor_brush_array *arr, Vector mins, Vector maxs,
                                int material, int is_entity)
{
  float t = 16.0f; // wall thickness

  // --- 1. Create the six wall brushes (unchanged) ---
  brush_t walls[6];
  int wallCount = 0;

  // Floor
  walls[wallCount++] = make_brush_cube(
      (Vector){mins.x, mins.y, mins.z},
      (Vector){maxs.x, mins.y + t, maxs.z});
  // Ceiling
  walls[wallCount++] = make_brush_cube(
      (Vector){mins.x, maxs.y - t, mins.z},
      (Vector){maxs.x, maxs.y, maxs.z});
  // North (+Z)
  walls[wallCount++] = make_brush_cube(
      (Vector){mins.x, mins.y, maxs.z - t},
      (Vector){maxs.x, maxs.y, maxs.z});
  // South (−Z)
  walls[wallCount++] = make_brush_cube(
      (Vector){mins.x, mins.y, mins.z},
      (Vector){maxs.x, maxs.y, mins.z + t});
  // East (+X)
  walls[wallCount++] = make_brush_cube(
      (Vector){maxs.x - t, mins.y, mins.z},
      (Vector){maxs.x, maxs.y, maxs.z});
  // West (−X)
  walls[wallCount++] = make_brush_cube(
      (Vector){mins.x, mins.y, mins.z},
      (Vector){mins.x + t, maxs.y, maxs.z});

  // --- 2. Create the inner void (the empty space) ---
  brush_t inner = make_brush_cube(
      (Vector){mins.x + t, mins.y + t, mins.z + t},
      (Vector){maxs.x - t, maxs.y - t, maxs.z - t});

  // --- 3. Subtract the inner void from each wall brush ---
  int totalPieces = 0;
  brush_t *finalPieces = NULL;

  for (int i = 0; i < 6; i++)
  {
    brush_t *pieces;
    int n = CSG_SubtractBrush(&walls[i], &inner, &pieces);
    if (n > 0)
    {
      finalPieces = realloc(finalPieces, sizeof(brush_t) * (totalPieces + n));
      memcpy(&finalPieces[totalPieces], pieces, sizeof(brush_t) * n);
      totalPieces += n;
      free(pieces);
    }
  }

  // --- 4. Add all resulting pieces to the editor array ---
  for (int i = 0; i < totalPieces; i++)
  {
    brush_t *b = &finalPieces[i];
    b->is_entity = is_entity;
    b->contents = CONTENTS_SOLID;
    b->dirty = 1;
    for (int s = 0; s < b->side_count; s++)
      b->sides[s].material_id = 1;
    arr->brushes[arr->count++] = *b;
  }

  free(finalPieces);
  printf("[EDITOR]: Room with CSG cleanup, %d piece(s) from 6 walls\n", totalPieces);
}
*/

void EditorCreate_BrushRoom_CSG(editor_brush_array *arr, Vector mins, Vector maxs,
                                int material, int is_entity)
{
  float t = 16.0f; // wall thickness
  brush_t outer = make_brush_cube(mins, maxs);
  brush_t inner = make_brush_cube(
      (Vector){mins.x + t, mins.y + t, mins.z + t},
      (Vector){maxs.x - t, maxs.y - t, maxs.z - t});

  brush_t *pieces;
  int n = CSG_SubtractBrush(&outer, &inner, &pieces);

  for (int i = 0; i < n; i++)
  {
    pieces[i].is_entity = is_entity;
    pieces[i].contents = CONTENTS_SOLID;
    pieces[i].dirty = 1;
    pieces[i].nodraw = 0;
    for (int s = 0; s < pieces[i].side_count; s++)
      pieces[i].sides[s].material_id = material;
    arr->brushes[arr->count++] = pieces[i];
  }
  free(pieces);
  printf("[EDITOR]: Hollow room via CSG, %d piece(s)\n", n);
}


