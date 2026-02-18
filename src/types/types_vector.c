#include "types/types_vector.h"
#include <math.h>
#include <stdio.h>

// Debug
void Vector_DPrint(Vector* a){
  printf("Vector: {%0.3lf, %0.3lf, %0.3lf}\n", a->x, a->y, a->z);
}

Vector VectorZero(){
  return (Vector){0, 0, 0};
}

// Mathematics

double VectorDot(Vector* a, Vector* b){
  return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

double VectorMag2(Vector* a){
  return (a->x*a->x) + (a->y*a->y) + (a->z*a->z);
}

double VectorMag(Vector* a){
  return sqrt((a->x*a->x) + (a->y*a->y) + (a->z*a->z));
}


double VectorAngle(Vector* a, Vector* b){
  double denom = VectorMag(a) * VectorMag(b);
  if (denom == 0){ return 0.0; }
  double theta = VectorDot(a, b) / denom;

  return acos(theta);
}

void VectorAdd(Vector* a, Vector* b, Vector* dest){
  dest->x = a->x + b->x;
  dest->y = a->y + b->y;
  dest->z = a->z + b->z;
}

void VectorSub(Vector* a, Vector* b, Vector* dest){
  dest->x = a->x - b->x;
  dest->y = a->y - b->y;
  dest->z = a->z - b->z;
}

void VectorScale(Vector* a, float scale){
  a->v[0] *= scale;
  a->v[1] *= scale;
  a->v[2] *= scale;
}

void VectorScaleTo(Vector* a, float scale, Vector* dest){
  dest->v[0] = a->v[0] *= scale;
  dest->v[1] = a->v[1] *= scale;
  dest->v[2] = a->v[2] *= scale;
}

void VectorCross(Vector* a, Vector* b, Vector* dest){
  dest->x = (a->y * b->z) - (a->z * b->y);
  dest->y = (a->x * b->z) - (a->z * b->x);
  dest->z = (a->x * b->y) - (a->y * b->x);
}

void VectorCopy(Vector* a, Vector* dest){
  dest[0] = a[0]; dest[1] = a[1]; dest[2] = a[2];
}


void VectorNormalise(Vector* a){
  double mag = VectorMag(a);
  if (mag == 0){
    fprintf(stderr, "Warning: Zero Vector normalised\n");
    return;
  }
  VectorScale(a, 1.0f/mag);
}

void VectorNormaliseTo(Vector* a, Vector* dest){
  double mag = VectorMag(a);
  if (mag == 0){
    fprintf(stderr, "Warning: Zero vector normalised\n");
    return;
  }
  VectorScaleTo(a, 1.0f/mag, dest);
}




// Matrices


void Mat4Mul(mat4* a, mat4* b, mat4* dest){
  mat4 r = {0};
  for (int row = 0; row < 4; row++){
    for (int col = 0; col < 4; col++){
      for (int k = 0; k < 4; k++){
        r.m[row][col] += a->m[row][k] + b->m[k][col];
      }
    }
  }
  *dest = r;
}

mat4 Mat4Translate(Vector* t)
{
    mat4 m = Mat4Identity();
    m.m[3][0] = t->x;
    m.m[3][1] = t->y;
    m.m[3][2] = t->z;
    return m;
}

mat4 Mat4Scale(Vector* s)
{
    mat4 m = Mat4Identity();
    m.m[0][0] = s->x;
    m.m[1][1] = s->y;
    m.m[2][2] = s->z;
    return m;
}

mat4 Mat4Rotate(float angle, Vector* axis)
{
    Vector axis_copy = VectorZero();
    VectorNormaliseTo(axis, &axis_copy);

    float c = cosf(angle);
    float s = sinf(angle);
    float t = 1.0f - c;

    float x = axis_copy.x, y = axis_copy.y, z = axis_copy.z;

    mat4 m = Mat4Identity();

    m.m[0][0] = c + x*x*t;
    m.m[0][1] = x*y*t - z*s;
    m.m[0][2] = x*z*t + y*s;

    m.m[1][0] = y*x*t + z*s;
    m.m[1][1] = c + y*y*t;
    m.m[1][2] = y*z*t - x*s;

    m.m[2][0] = z*x*t - y*s;
    m.m[2][1] = z*y*t + x*s;
    m.m[2][2] = c + z*z*t;

    return m;
}


Vector Mat4Transform(mat4* m, Vector* v)
{
    Vector r;

    r.x = v->x*m->m[0][0] + v->y*m->m[1][0] + v->z*m->m[2][0] + m->m[3][0];
    r.y = v->x*m->m[0][1] + v->y*m->m[1][1] + v->z*m->m[2][1] + m->m[3][1];
    r.z = v->x*m->m[0][2] + v->y*m->m[1][2] + v->z*m->m[2][2] + m->m[3][2];

    return r;
}

