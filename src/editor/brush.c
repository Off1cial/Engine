#include "editor/brush.h"
#include "mem.h"


void EditorBrushArray_Init(editor_brush_array* arr, size_t capacity){
  arr->brushes = malloc(sizeof(brush_t) * capacity);
  if (!arr->brushes){
    fprintf(stderr, "[EDITOR]: Failed to init brush array\n");
    exit(1);
  }

  arr->count = 0;
  arr->capacity = capacity;
}


void EditorBrushArray_Destroy(editor_brush_array* arr){
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
    v + s->uv_origin.y
  };
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

  b.pos = VectorScale( VectorAdd(mins, maxs), 0.5f);
  b.scale = VectorSub(maxs, mins);

  return b;
}
winding_t base_winding(plane_t p)
{
  winding_t w = {0};

  Vector up = (fabsf(p.normal.y) < 0.9f) ? VECTOR_AXIS_Y : VECTOR_AXIS_X;

  Vector u = VectorCrossNormalise(up, p.normal);
  Vector v = VectorCrossNormalise(p.normal, u);

  float size = 300.0f;

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

winding_t clip_winding(winding_t *in, plane_t p)
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

mesh_t BrushToMesh(brush_t *b)
{
  mesh_t m = {0};
  MeshInit(&m, 24, 24);

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

    for (int v = 1; v < base.count - 1; v++)
    {
      Vector p0 = base.v[0];
      Vector p1 = base.v[v];
      Vector p2 = base.v[v + 1];

      Vector2 uv0 = Brush_ComputeUV(&b->sides[i], p0);
      Vector2 uv1 = Brush_ComputeUV(&b->sides[i], p1);
      Vector2 uv2 = Brush_ComputeUV(&b->sides[i], p2);

      GLuint i0 = MeshPushVertex(&m, MakeVertex(base.v[0], VectorScale(VECTOR_ONE, 0.8f), uv0));
      GLuint i1 = MeshPushVertex(&m, MakeVertex(base.v[v], VectorScale(VECTOR_ONE, 0.8f), uv1));
      GLuint i2 = MeshPushVertex(&m, MakeVertex(base.v[v + 1], VectorScale(VECTOR_ONE, 0.8f), uv2));

      MeshPushTriangle(&m, i0, i1, i2);
    }
  }

  return m;
}
void EditorBrush_Draw(brush_t* brush, rdrawqueue_t* drawlist, camera_t* cam){
  struct rcmd_t* cmd = MEM_ARENA_ALLOC(gMemArena, sizeof(struct rcmd_t), alignof(struct rcmd_t));
  cmd->type = RCMD_DRAW_MESH;
  cmd->draw_mesh.mesh = &brush->editor_mesh;
  cmd->draw_mesh.mode = GL_TRIANGLES;
  cmd->draw_mesh.model = Mat4Identity();
  cmd->draw_mesh.material = gRendererState->materials[0];

  RDrawQueue_Push(drawlist, cmd);
}


bool point_in_brush(brush_t* brush, Vector p){
  if (!brush) return false;

  for(int i = 0; i < brush->side_count; i++){
    brush_side_t* side = &brush->sides[i];

    float d = VectorDot(p, side->plane.normal) - side->plane.dist;

    if (d > 0.1f){
      return false;
    }
  }

  return true;
}

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

    float tmin = -__FLT_MAX__;
    float tmax = __FLT_MAX__;

    int enter_side = -1;

    for (int i = 0; i < brush->side_count; i++)
    {
        plane_t* p = &brush->sides[i].plane;

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
    *out_hit  = VectorAdd(ray.origin, VectorScale(ray.dir, tmin));

    return true;
}
