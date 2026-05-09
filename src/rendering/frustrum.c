#include "rendering/frustum.h"

static plane_t ExtractPlane(mat4 m, int row, int sign)
{
  plane_t p;

  p.normal.x = m.m[0][3] + sign * m.m[0][row];
  p.normal.y = m.m[1][3] + sign * m.m[1][row];
  p.normal.z = m.m[2][3] + sign * m.m[2][row];

  p.dist = m.m[3][3] + sign * m.m[3][row];

  float len = VectorMag(p.normal);

  p.normal = VectorScale(p.normal, 1.0f / len);
  p.dist /= len;

  return p;
}

void FrustumBuild(frustum_t *f, mat4 vp)
{
  f->planes[FRUSTUM_LEFT] = ExtractPlane(vp, 0, 1);
  f->planes[FRUSTUM_RIGHT] = ExtractPlane(vp, 0, -1);

  f->planes[FRUSTUM_BOTTOM] = ExtractPlane(vp, 1, 1);
  f->planes[FRUSTUM_TOP] = ExtractPlane(vp, 1, -1);

  f->planes[FRUSTUM_NEAR] = ExtractPlane(vp, 2, 1);
  f->planes[FRUSTUM_FAR] = ExtractPlane(vp, 2, -1);
}

bool frustum_contains_sphere(frustum_t* f, Vector centre, float radius){
  for (int i = 0; i < 6; i++){
    plane_t* p = &f->planes[i];

    float d = VectorDot(p->normal, centre) + p->dist;

    if (d < -radius) return false;
  }
  return true;
}