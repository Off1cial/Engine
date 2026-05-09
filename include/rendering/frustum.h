#ifndef FRUSTRUM_H
#define FRUSTRUM_H

#include "types/types_vector.h"

enum
{
  FRUSTUM_LEFT,
  FRUSTUM_RIGHT,
  FRUSTUM_BOTTOM,
  FRUSTUM_TOP,
  FRUSTUM_NEAR,
  FRUSTUM_FAR
};


typedef struct
{
  plane_t planes[6];
} frustum_t;

static plane_t ExtractPlane(mat4 m, int row, int sign);
void FrustumBuild(frustum_t *f, mat4 vp);
bool frustum_contains_sphere(frustum_t* f, Vector centre, float radius);

#endif