#include "types/types_vector.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

Vector VECTOR_AXES[6] = {
  VECTOR_AXIS_X,
  (Vector){-1, 0, 0},
  VECTOR_AXIS_Y,
  (Vector){0, -1, 0},
  VECTOR_AXIS_Z,
  (Vector){0, 0, -1}
};




// Debug
void Vector_DPrint(Vector* a){
  printf("Vector: {%0.3lf, %0.3lf, %0.3lf}\n", a->x, a->y, a->z);
}

Vector VectorZero(){
  return (Vector){0, 0, 0};
}

Vector VectorInit(float x, float y, float z){
  return (Vector){x, y, z};
}
// Mathematics

double VectorDot(Vector a, Vector b){
  return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

double VectorMag2(Vector a){
  return (a.x*a.x) + (a.y*a.y) + (a.z*a.z);
}

double VectorMag(Vector a){
  return sqrt((a.x*a.x) + (a.y*a.y) + (a.z*a.z));
}


double VectorAngle(Vector a, Vector b){
  double denom = VectorMag(a) * VectorMag(b);
  if (denom == 0){ return 0.0; }
  double theta = VectorDot(a, b) / denom;

  return acos(theta);
}

Vector VectorAdd(Vector a, Vector b){
  return VectorInit(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vector VectorSub(Vector a, Vector b){
  return VectorInit(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vector VectorScale(Vector a, float scale){
  return (Vector){
    a.x * scale,
    a.y * scale,
    a.z * scale
  };
}

void VectorScaleInPlace(Vector* a, float scale){
  a->v[0] *= scale;
  a->v[1] *= scale;
  a->v[2] *= scale;
}

void VectorScaleTo(Vector a, float scale, Vector* dest){
  dest->v[0] = a.v[0] * scale;
  dest->v[1] = a.v[1] * scale;
  dest->v[2] = a.v[2] * scale;
}



void VectorNormalise(Vector* a){
  double mag = VectorMag(*a);
  if (mag == 0){
    fprintf(stderr, "Warning: Zero Vector normalised\n");
    return;
  }
  VectorScaleInPlace(a, 1.0f/mag);
}

void VectorNormaliseTo(Vector a, Vector* dest){
  double mag = VectorMag(a);
  if (mag == 0){
    fprintf(stderr, "Warning: Zero vector normalised\n");
    return;
  }
  VectorScaleTo(a, 1.0f/mag, dest);
}



Vector VectorCross(Vector a, Vector b){
  float x = (a.y * b.z) - (a.z * b.y);
  float y = (a.x * b.z) - (a.z * b.x);
  float z = (a.x * b.y) - (a.y * b.x);
  return VectorInit(x, y, z);
}

Vector VectorCrossNormalise(Vector a, Vector b){
  Vector cross =  VectorCross(a, b);
  VectorNormalise(&cross);
  return cross;
}

void VectorCopy(Vector a, Vector* dest){
  dest->v[0] = a.v[0]; dest->v[1] = a.v[1]; dest->v[2] = a.v[2];
}




// Matrices
void Mat4Copy(const mat4 src, mat4* dest){
  memcpy(dest->m, src.m, sizeof(float)*16);
}

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

mat4 Mat4Rotate(float angle, Vector axis)
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


mat4 Mat4LookAt(Vector eye, Vector center, Vector up)
{
    Vector f, r, u;
    f = VectorSub(center, eye);
    VectorNormalise(&f);

    r = VectorCross(f, up);
    VectorNormalise(&r);

    u = VectorCross(r, f);

    mat4 view = Mat4Identity();

    view.m[0][0] = r.x;
    view.m[1][0] = r.y;
    view.m[2][0] = r.z;

    view.m[0][1] = u.x;
    view.m[1][1] = u.y;
    view.m[2][1] = u.z;

    view.m[0][2] = -f.x;
    view.m[1][2] = -f.y;
    view.m[2][2] = -f.z;

    view.m[3][0] = -VectorDot(r, eye);
    view.m[3][1] = -VectorDot(u, eye);
    view.m[3][2] =  VectorDot(f, eye);

    return view;
}

mat4 Mat4Perspective(float fovY, float aspect, float znear, float zfar)
{
    float f = 1.0f / tanf(fovY * 0.5f); // f = cot(fovY/2)
    mat4 m = {0};

    m.m[0][0] = f / aspect;
    m.m[1][1] = f;
    m.m[2][2] = (zfar + znear) / (znear - zfar);
    m.m[2][3] = -1.0f;
    m.m[3][2] = (2.0f * zfar * znear) / (znear - zfar);

    return m;
}

