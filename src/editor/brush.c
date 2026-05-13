#include "editor/brush.h"
#include "mem.h"

void EditorBrushArray_Init(editor_brush_array *arr, size_t capacity)
{
  arr->brushes = malloc(sizeof(brush_t) * capacity);
  if (!arr->brushes)
  {
    fprintf(stderr, "[EDITOR]: Failed to init brush array\n");
    exit(1);
  }

  arr->count = 0;
  arr->capacity = capacity;

  arr->hovered_side = (brush_side_hovered_t){0};
  arr->hovered_side.dirty = 1;
  arr->hovered_side.side = NULL;
  arr->hovered_side.owner_brush = NULL;
  arr->hovered_side.material = gRendererState->materials[1];

  MeshInit(&arr->hovered_side.mesh, 24, 24);
}

void EditorBrushArray_Destroy(editor_brush_array *arr)
{
  free(arr->brushes);
  arr->count = arr->capacity = 0;
}

static void BrushSide_BuildUVBasis(brush_side_t *s)
{
  Vector n = s->plane.normal;

  Vector up = (fabsf(n.y) < 0.9f) ? VECTOR_AXIS_Y : VECTOR_AXIS_X;

  s->uv_axis_u = VectorCrossNormalise(up, n);
  s->uv_axis_v = VectorCrossNormalise(n, s->uv_axis_u);

  s->uv_origin = (Vector2){0, 0};
}

static Vector2 Brush_ComputeUV(brush_side_t *s, Vector p)
{
  const float scale = 0.01f; // default scale
  float u = VectorDot(p, s->uv_axis_u) * scale;
  float v = VectorDot(p, s->uv_axis_v) * scale;

  return (Vector2){
      u + s->uv_origin.x,
      v + s->uv_origin.y};
}

static void BrushSide_DefaultUVs(brush_side_t *s)
{
  Vector n = s->plane.normal;

  s->uv_origin = (Vector2){0, 0};

  if (fabsf(n.x) > 0.9f)
  {
    s->uv_axis_u = VECTOR_AXIS_Y;
    s->uv_axis_v = VECTOR_AXIS_Z;
  }
  else if (fabsf(n.y) > 0.9f)
  {
    s->uv_axis_u = VECTOR_AXIS_X;
    s->uv_axis_v = VECTOR_AXIS_Z;
  }
  else
  {
    s->uv_axis_u = VECTOR_AXIS_X;
    s->uv_axis_v = VECTOR_AXIS_Y;
  }
}

brush_t make_brush_cube(Vector mins, Vector maxs)
{
  brush_t b = {0};

  b.side_count = 6;

  b.sides[0].plane = (plane_t){{1, 0, 0}, maxs.x};
  b.sides[1].plane = (plane_t){{-1, 0, 0}, -mins.x};

  b.sides[2].plane = (plane_t){{0, 1, 0}, maxs.y};
  b.sides[3].plane = (plane_t){{0, -1, 0}, -mins.y};

  b.sides[4].plane = (plane_t){{0, 0, 1}, maxs.z};
  b.sides[5].plane = (plane_t){{0, 0, -1}, -mins.z};

  BrushSide_DefaultUVs(&b.sides[0]);
  BrushSide_DefaultUVs(&b.sides[1]);
  BrushSide_DefaultUVs(&b.sides[2]);
  BrushSide_DefaultUVs(&b.sides[3]);
  BrushSide_DefaultUVs(&b.sides[4]);
  BrushSide_DefaultUVs(&b.sides[5]);

  b.pos = VectorScale(VectorAdd(mins, maxs), 0.5f);
  b.scale = VectorSub(maxs, mins);

  return b;
}



void BrushHoveredSideComputeMesh(brush_side_hovered_t *hside)
{

  MeshReset(&hside->mesh);
  mesh_t *m = &hside->mesh;

  if (!hside || !hside->side || !hside->owner_brush)
  {
    return;
  }

  brush_t *brush = hside->owner_brush;
  plane_t *plane = &hside->side->plane;

  winding_t base = base_winding(hside->side->plane);

  for (size_t i = 0; i < brush->side_count; i++)
  {
    if (&brush->sides[i] == hside->side)
    {
      continue;
    }
    base = clip_winding(&base, brush->sides[i].plane);
  }

  printf("hovered winding of %d points\n", base.count);

  const float vertex_offset = 0.08f;
  VectorNormalise(&plane->normal);
  Vector vector_offset = VectorScale(plane->normal, vertex_offset);

  // Vertex creation
  for (size_t i = 1; i < base.count - 1; i++)
  {
    Vector p0 = VectorAdd(base.v[0], vector_offset);
    Vector p1 = VectorAdd(base.v[i], vector_offset);
    Vector p2 = VectorAdd(base.v[i + 1], vector_offset);

    Vector2 uv0 = Brush_ComputeUV(hside->side, p0);
    Vector2 uv1 = Brush_ComputeUV(hside->side, p1);
    Vector2 uv2 = Brush_ComputeUV(hside->side, p2);

    GLuint i0 = MeshPushVertex(m, MakeVertex(p0, VectorScale(VECTOR_ONE, 0.8f), uv0));
    GLuint i1 = MeshPushVertex(m, MakeVertex(p1, VectorScale(VECTOR_ONE, 0.8f), uv1));
    GLuint i2 = MeshPushVertex(m, MakeVertex(p2, VectorScale(VECTOR_ONE, 0.8f), uv2));

    MeshPushTriangle(m, i0, i1, i2);
  }
}

void BrushToMesh(brush_t *b, mesh_t* mesh_out)
{
  MeshReset(mesh_out);
  MeshInit(mesh_out, 24, 24);

  b->edge_count = 0;

  for (int i = 0; i < b->side_count; i++)
  {
    winding_t base = base_winding(b->sides[i].plane);

    if (base.count < 3)
      continue;

    for (int j = 0; j < b->side_count; j++)
    {
      if (j == i)
        continue;

      base = clip_winding(&base, b->sides[j].plane);
    }

    for (int e = 0; e < base.count; e++)
    {
        if (b->edge_count >= MAX_BRUSH_EDGES)
            break;

        Vector a = base.v[e];
        Vector bpos = base.v[(e + 1) % base.count];

        brush_edge_t* edge =
            &b->edges[b->edge_count++];

        edge->a = a;
        edge->b = bpos;

        edge->side_a = &b->sides[i];
        edge->side_b = NULL;
    }

    for (int v = 1; v < base.count - 1; v++)
    {
      Vector p0 = base.v[0];
      Vector p1 = base.v[v];
      Vector p2 = base.v[v + 1];

      Vector2 uv0 = Brush_ComputeUV(&b->sides[i], p0);
      Vector2 uv1 = Brush_ComputeUV(&b->sides[i], p1);
      Vector2 uv2 = Brush_ComputeUV(&b->sides[i], p2);

      struct vertex_t v0 = {
        .pos = base.v[0],
        .colour = VECTOR_ONE,
        .uv = uv0,
        .tangent = b->sides[i].uv_axis_u
      };

      struct vertex_t v1 = {
        .pos = base.v[v],
        .colour = VECTOR_ONE,
        .uv = uv1,
        .tangent = b->sides[i].uv_axis_u
      };


      struct vertex_t v2 = {
        .pos = base.v[v+1],
        .colour = VECTOR_ONE,
        .uv = uv2,
        .tangent = b->sides[i].uv_axis_u
      };

      
      /*
      GLuint i0 = MeshPushVertex(mesh_out, MakeVertex(base.v[0], VectorScale(VECTOR_ONE, 0.8f), uv0));
      GLuint i1 = MeshPushVertex(mesh_out, MakeVertex(base.v[v], VectorScale(VECTOR_ONE, 0.8f), uv1));
      GLuint i2 = MeshPushVertex(mesh_out, MakeVertex(base.v[v + 1], VectorScale(VECTOR_ONE, 0.8f), uv2));
      */
      
      GLuint i0 = MeshPushVertex(mesh_out, v0);
      GLuint i1 = MeshPushVertex(mesh_out, v1);
      GLuint i2 = MeshPushVertex(mesh_out, v2);
      

      MeshPushTriangle(mesh_out, i0, i1, i2);
    }
  }
}
void EditorBrush_Draw(brush_t *brush, rdrawqueue_t *drawlist, camera_t *cam)
{
  if (brush->dirty){
    MeshReset(&brush->editor_mesh);
    BrushToMesh(brush, &brush->editor_mesh);
    MeshRecalculateNormals(&brush->editor_mesh);
    MeshUpload(&brush->editor_mesh, GL_STATIC_DRAW);
    brush->dirty = 0;
  }



  struct rcmd_t *cmd = MEM_ARENA_ALLOC(gMemArena, sizeof(struct rcmd_t), alignof(struct rcmd_t));
  cmd->type = RCMD_DRAW_MESH;
  cmd->draw_mesh.mesh = &brush->editor_mesh;
  cmd->draw_mesh.mode = (!gRendererState->wireframe) ? GL_TRIANGLES : GL_LINES;
  cmd->draw_mesh.model = Mat4Identity();
  cmd->draw_mesh.material = gRendererState->materials[0];

  RDrawQueue_Push(drawlist, cmd);
}

void EditorBrush_DrawHoveredSide(brush_side_hovered_t *hside)
{

  if (!hside)
  {
    return;
  }
  if (!hside->side)
  {
    return;
  }
  if (!hside->owner_brush)
  {
    return;
  }

  if (hside->dirty)
  {
    BrushHoveredSideComputeMesh(hside);
    MeshRecalculateNormals(&hside->mesh);
    MeshUpload(&hside->mesh, GL_STATIC_DRAW);
    MeshDebug_PrintVertices(&hside->mesh);
    printf("Hovered mesh created\n");
    hside->dirty = 0;
  }

  struct rcmd_t *cmd = MEM_ARENA_ALLOC(gMemArena, sizeof(struct rcmd_t), alignof(struct rcmd_t));

  cmd->type = RCMD_DRAW_MESH;
  cmd->draw_mesh.mesh = &hside->mesh;
  cmd->draw_mesh.mode = GL_TRIANGLES;
  cmd->draw_mesh.model = Mat4Identity();
  cmd->draw_mesh.material = gRendererState->materials[1];

  RDrawQueue_Push(gRendererState->draw_q, cmd);
}

bool point_in_brush(brush_t *brush, Vector p)
{
  if (!brush)
    return false;

  for (int i = 0; i < brush->side_count; i++)
  {
    brush_side_t *side = &brush->sides[i];

    float d = VectorDot(p, side->plane.normal) - side->plane.dist;

    if (d > 0.1f)
    {
      return false;
    }
  }

  return true;
}

/*
bool Brush_Raycast(
    brush_t* brush,
    int* out_side,
    Vector* out_hit,
    float* out_dist,
    camera_t* cam,
    float mx,
    float my)
{
    ray_t ray = Camera_ScreenPointToRay(cam, mx, my);

    printf("ray dir: %f %f %f\n",
    ray.dir.x,
    ray.dir.y,
    ray.dir.z);

Vector local_origin = VectorSub(ray.origin, brush->pos);

float tmin = -__FLT_MAX__;
float tmax = __FLT_MAX__;

int enter_side = -1;

for (int i = 0; i < brush->side_count; i++)
{
  plane_t *p = &brush->sides[i].plane;

  float denom = VectorDot(p->normal, ray.dir);

  // correct signed distance (FIX #1)
  float dist = VectorDot(p->normal, ray.origin) - p->dist;

  if (fabsf(denom) < 1e-6f)
  {
    if (dist > 0.0f)
      return false;
    continue;
  }

  float t = -dist / denom;

  if (denom < 0.0f)
  {
    // entering plane
    if (t > tmin)
    {
      tmin = t;
      enter_side = i;
    }
  }
  else
  {
    // exiting plane
    if (t < tmax)
      tmax = t;
  }

  if (tmin > tmax)
    return false;
}

if (enter_side == -1)
  return false;

*out_side = enter_side;
*out_dist = tmin;
*out_hit = VectorAdd(ray.origin, VectorScale(ray.dir, tmin));

return true;
}

*/

bool Brush_Raycast(
    brush_t *brush,
    int *out_side,
    Vector *out_hit,
    float *out_dist,
    camera_t *cam,
    float mx,
    float my)
{
  ray_t ray = Camera_ScreenPointToRay(cam, mx, my);

  float bestDist = __FLT_MAX__;
  int bestSide = -1;
  Vector bestHit = VECTOR_ZERO;

  for (int i = 0; i < brush->side_count; i++)
  {
    plane_t *p = &brush->sides[i].plane;

    float denom = VectorDot(p->normal, ray.dir);

    // ignore nearly parallel
    if (fabsf(denom) < 0.0001f)
      continue;

    float t =
        (p->dist - VectorDot(p->normal, ray.origin)) / denom;

    // behind camera
    if (t < 0.0f)
      continue;

    Vector hit =
        VectorAdd(ray.origin,
                  VectorScale(ray.dir, t));

    // test if hit lies inside all planes
    bool inside = true;

    for (int j = 0; j < brush->side_count; j++)
    {
      plane_t *clip = &brush->sides[j].plane;

      float d =
          VectorDot(clip->normal, hit) - clip->dist;

      if (d > 0.01f)
      {
        inside = false;
        break;
      }
    }

    if (!inside)
      continue;

    if (t < bestDist)
    {
      bestDist = t;
      bestSide = i;
      bestHit = hit;
    }
  }

  if (bestSide == -1)
    return false;

  *out_side = bestSide;
  *out_dist = bestDist;
  *out_hit = bestHit;

  return true;
}
