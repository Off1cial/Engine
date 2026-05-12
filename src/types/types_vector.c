#include "types/types_vector.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#ifndef EPSILON
#define EPSILON 1e-6f
#endif

/* ============================================================
   AXES
   ============================================================ */

Vector VECTOR_AXES[6] = {
    VECTOR_AXIS_X,
    VECTOR_AXIS_X_NEG,
    VECTOR_AXIS_Y,
    VECTOR_AXIS_Y_NEG,
    VECTOR_AXIS_Z,
    VECTOR_AXIS_Z_NEG
};

/* ============================================================
   DEBUG
   ============================================================ */

void Vector_DPrint(Vector* a)
{
    printf("Vector: {%0.3f, %0.3f, %0.3f}\n", a->x, a->y, a->z);
}

/* ============================================================
   BASIC CONSTRUCTORS
   ============================================================ */

Vector VectorZero()
{
    return (Vector){0, 0, 0};
}

Vector VectorInit(float x, float y, float z)
{
    return (Vector){x, y, z};
}

/* ============================================================
   VECTOR MATH
   ============================================================ */

double VectorDot(Vector a, Vector b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

double VectorMag2(Vector a)
{
    return VectorDot(a, a);
}

double VectorMag(Vector a)
{
    return sqrt(VectorMag2(a));
}

double VectorAngle(Vector a, Vector b)
{
    double mag = VectorMag(a) * VectorMag(b);
    if (mag < EPSILON)
        return 0.0;

    double c = VectorDot(a, b) / mag;

    if (c > 1.0) c = 1.0;
    if (c < -1.0) c = -1.0;

    return acos(c);
}

Vector VectorAdd(Vector a, Vector b)
{
    return VectorInit(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vector VectorSub(Vector a, Vector b)
{
    return VectorInit(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vector VectorScale(Vector a, float scale)
{
    return VectorInit(a.x * scale, a.y * scale, a.z * scale);
}

void VectorScaleInPlace(Vector* a, float scale)
{
    a->x *= scale;
    a->y *= scale;
    a->z *= scale;
}

void VectorScaleTo(Vector a, float scale, Vector* dest)
{
    dest->x = a.x * scale;
    dest->y = a.y * scale;
    dest->z = a.z * scale;
}

Vector VectorNegate(Vector a){
    return (Vector){-a.x, -a.y, -a.z};
}

void VectorNegateInPlace(Vector* a)
{
    a->x = -a->x;
    a->y = -a->y;
    a->z = -a->z;
}

void VectorNegateTo(Vector a, Vector* dest)
{
    dest->x = -a.x;
    dest->y = -a.y;
    dest->z = -a.z;
}

void VectorNormalise(Vector* a)
{
    double mag = VectorMag(*a);
    if (mag < EPSILON)
    {
        fprintf(stderr, "Warning: normalising zero vector\n");
        return;
    }

    VectorScaleInPlace(a, 1.0f / (float)mag);
}

void VectorNormaliseTo(Vector a, Vector* dest)
{
    *dest = a;
    VectorNormalise(dest);
}

Vector VectorCross(Vector a, Vector b)
{
    return VectorInit(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

Vector VectorCrossNormalise(Vector a, Vector b)
{
    Vector r = VectorCross(a, b);
    VectorNormalise(&r);
    return r;
}

void VectorCopy(Vector a, Vector* dest)
{
    dest->x = a.x;
    dest->y = a.y;
    dest->z = a.z;
}

int VectorEqual(Vector a, Vector b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

int VectorCompare_Imprecise(Vector a, Vector b, float min, float max)
{
    (void)min;
    (void)max;

    const float eps = 0.001f;

    return fabsf(a.x - b.x) < eps &&
           fabsf(a.y - b.y) < eps &&
           fabsf(a.z - b.z) < eps;
}



float Vector2Cross(Vector2 o, Vector2 a, Vector2 b){
  return
    (a.x - b.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

Vector2 Vector2Add(Vector2 a, Vector2 b){
    return (Vector2){a.x + b.x, a.y + b.y};
}

Vector2 Vector2Scale(Vector2 v, float scale){
    return (Vector2){v.x * scale, v.y * scale};
}

/* ============================================================
   MATRIX HELPERS (COLUMN MAJOR)
   ============================================================ */


void Mat4Copy(const mat4 src, mat4* dest)
{
    memcpy(dest->m, src.m, sizeof(float) * 16);
}

mat4 Mat4Mul(mat4* a, mat4* b)
{
    mat4 r = {0};

    for (int col = 0; col < 4; col++)
        for (int row = 0; row < 4; row++)
            for (int k = 0; k < 4; k++)
                r.m[col][row] += a->m[k][row] * b->m[col][k];

    return r;
}

void Mat4MulTo(mat4* a, mat4* b, mat4* dest)
{
    *dest = Mat4Mul(a, b);
}

Vector4 Mat4Mulv(mat4* m, Vector4* v){
  #define M(c , r) (m->m[c][r])

  Vector4 out = {0, 0, 0, 0};

  for (int r = 0; r < 4; r++){
    out.v[r] =  
      M(0, r) * v->x +
      M(1, r) * v->y +
      M(2, r) * v->z + 
      M(3, r) * v->w;
  }


  #undef M

  return out;
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
    VectorNormalise(&axis);

    float c = cosf(angle);
    float s = sinf(angle);
    float t = 1.0f - c;

    float x = axis.x;
    float y = axis.y;
    float z = axis.z;

    mat4 m = Mat4Identity();

    m.m[0][0] = c + x*x*t;
    m.m[0][1] = x*y*t + z*s;
    m.m[0][2] = x*z*t - y*s;

    m.m[1][0] = y*x*t - z*s;
    m.m[1][1] = c + y*y*t;
    m.m[1][2] = y*z*t + x*s;

    m.m[2][0] = z*x*t + y*s;
    m.m[2][1] = z*y*t - x*s;
    m.m[2][2] = c + z*z*t;

    return m;
}

Vector Mat4Transform(mat4* m, Vector* v)
{
    Vector r;

    r.x = v->x * m->m[0][0] + v->y * m->m[1][0] + v->z * m->m[2][0] + m->m[3][0];
    r.y = v->x * m->m[0][1] + v->y * m->m[1][1] + v->z * m->m[2][1] + m->m[3][1];
    r.z = v->x * m->m[0][2] + v->y * m->m[1][2] + v->z * m->m[2][2] + m->m[3][2];

    return r;
}

/* ============================================================
   CAMERA MATRICES
   ============================================================ */

mat4 Mat4LookAt(Vector eye, Vector center, Vector up)
{
    Vector back = VectorSub(eye, center);
    VectorNormalise(&back);

    Vector r = VectorCrossNormalise(up, back);

    Vector u = VectorCross(back, r);

    mat4 view = Mat4Identity();

    view.m[0][0] = r.x;
    view.m[0][1] = u.x;
    view.m[0][2] = back.x;

    view.m[1][0] = r.y;
    view.m[1][1] = u.y;
    view.m[1][2] = back.y;

    view.m[2][0] = r.z;
    view.m[2][1] = u.z;
    view.m[2][2] = back.z;

    view.m[3][0] = -VectorDot(r, eye);
    view.m[3][1] = -VectorDot(u, eye);
    view.m[3][2] =  -VectorDot(back, eye);

    return view;
}

mat4 Mat4Perspective(float fovY, float aspect, float znear, float zfar)
{
    float f = 1.0f / tanf(fovY * 0.5f);

    mat4 m = {0};

    m.m[0][0] = f / aspect;
    m.m[1][1] = f;
    m.m[2][2] = (zfar + znear) / (znear - zfar);
    m.m[2][3] = -1.0f;
    m.m[3][2] = (2.0f * zfar * znear) / (znear - zfar);

    return m;
}

bool Mat4Inverse(mat4* m, mat4* out)
{
    float inv[16];
    float det;

    // Flatten access helper
    #define M(c,r) m->m[c][r]
    #define O(c,r) out->m[c][r]

    inv[ 0] =  M(1,1)*M(2,2)*M(3,3) - M(1,1)*M(2,3)*M(3,2)
             - M(2,1)*M(1,2)*M(3,3) + M(2,1)*M(1,3)*M(3,2)
             + M(3,1)*M(1,2)*M(2,3) - M(3,1)*M(1,3)*M(2,2);

    inv[ 4] = -M(0,1)*M(2,2)*M(3,3) + M(0,1)*M(2,3)*M(3,2)
             + M(2,1)*M(0,2)*M(3,3) - M(2,1)*M(0,3)*M(3,2)
             - M(3,1)*M(0,2)*M(2,3) + M(3,1)*M(0,3)*M(2,2);

    inv[ 8] =  M(0,1)*M(1,2)*M(3,3) - M(0,1)*M(1,3)*M(3,2)
             - M(1,1)*M(0,2)*M(3,3) + M(1,1)*M(0,3)*M(3,2)
             + M(3,1)*M(0,2)*M(1,3) - M(3,1)*M(0,3)*M(1,2);

    inv[12] = -M(0,1)*M(1,2)*M(2,3) + M(0,1)*M(1,3)*M(2,2)
             + M(1,1)*M(0,2)*M(2,3) - M(1,1)*M(0,3)*M(2,2)
             - M(2,1)*M(0,2)*M(1,3) + M(2,1)*M(0,3)*M(1,2);

    inv[ 1] = -M(1,0)*M(2,2)*M(3,3) + M(1,0)*M(2,3)*M(3,2)
             + M(2,0)*M(1,2)*M(3,3) - M(2,0)*M(1,3)*M(3,2)
             - M(3,0)*M(1,2)*M(2,3) + M(3,0)*M(1,3)*M(2,2);

    inv[ 5] =  M(0,0)*M(2,2)*M(3,3) - M(0,0)*M(2,3)*M(3,2)
             - M(2,0)*M(0,2)*M(3,3) + M(2,0)*M(0,3)*M(3,2)
             + M(3,0)*M(0,2)*M(2,3) - M(3,0)*M(0,3)*M(2,2);

    inv[ 9] = -M(0,0)*M(1,2)*M(3,3) + M(0,0)*M(1,3)*M(3,2)
             + M(1,0)*M(0,2)*M(3,3) - M(1,0)*M(0,3)*M(3,2)
             - M(3,0)*M(0,2)*M(1,3) + M(3,0)*M(0,3)*M(1,2);

    inv[13] =  M(0,0)*M(1,2)*M(2,3) - M(0,0)*M(1,3)*M(2,2)
             - M(1,0)*M(0,2)*M(2,3) + M(1,0)*M(0,3)*M(2,2)
             + M(2,0)*M(0,2)*M(1,3) - M(2,0)*M(0,3)*M(1,2);

    inv[ 2] =  M(1,0)*M(2,1)*M(3,3) - M(1,0)*M(2,3)*M(3,1)
             - M(2,0)*M(1,1)*M(3,3) + M(2,0)*M(1,3)*M(3,1)
             + M(3,0)*M(1,1)*M(2,3) - M(3,0)*M(1,3)*M(2,1);

    inv[ 6] = -M(0,0)*M(2,1)*M(3,3) + M(0,0)*M(2,3)*M(3,1)
             + M(2,0)*M(0,1)*M(3,3) - M(2,0)*M(0,3)*M(3,1)
             - M(3,0)*M(0,1)*M(2,3) + M(3,0)*M(0,3)*M(2,1);

    inv[10] =  M(0,0)*M(1,1)*M(3,3) - M(0,0)*M(1,3)*M(3,1)
             - M(1,0)*M(0,1)*M(3,3) + M(1,0)*M(0,3)*M(3,1)
             + M(3,0)*M(0,1)*M(1,3) - M(3,0)*M(0,3)*M(1,1);

    inv[14] = -M(0,0)*M(1,1)*M(2,3) + M(0,0)*M(1,3)*M(2,1)
             + M(1,0)*M(0,1)*M(2,3) - M(1,0)*M(0,3)*M(2,1)
             - M(2,0)*M(0,1)*M(1,3) + M(2,0)*M(0,3)*M(1,1);

    inv[ 3] = -M(1,0)*M(2,1)*M(3,2) + M(1,0)*M(2,2)*M(3,1)
             + M(2,0)*M(1,1)*M(3,2) - M(2,0)*M(1,2)*M(3,1)
             - M(3,0)*M(1,1)*M(2,2) + M(3,0)*M(1,2)*M(2,1);

    inv[ 7] =  M(0,0)*M(2,1)*M(3,2) - M(0,0)*M(2,2)*M(3,1)
             - M(2,0)*M(0,1)*M(3,2) + M(2,0)*M(0,2)*M(3,1)
             + M(3,0)*M(0,1)*M(2,2) - M(3,0)*M(0,2)*M(2,1);

    inv[11] = -M(0,0)*M(1,1)*M(3,2) + M(0,0)*M(1,2)*M(3,1)
             + M(1,0)*M(0,1)*M(3,2) - M(1,0)*M(0,2)*M(3,1)
             - M(3,0)*M(0,1)*M(1,2) + M(3,0)*M(0,2)*M(1,1);

    inv[15] =  M(0,0)*M(1,1)*M(2,2) - M(0,0)*M(1,2)*M(2,1)
             - M(1,0)*M(0,1)*M(2,2) + M(1,0)*M(0,2)*M(2,1)
             + M(2,0)*M(0,1)*M(1,2) - M(2,0)*M(0,2)*M(1,1);

    det = M(0,0)*inv[0] + M(0,1)*inv[4] + M(0,2)*inv[8] + M(0,3)*inv[12];

    if (det == 0.0f)
        return false;

    det = 1.0f / det;

    for (int i = 0; i < 16; i++)
        inv[i] *= det;

    // Write back into column-major mat4
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            O(c,r) = inv[c*4 + r];

    return true;

    #undef M
    #undef O
}
/* ============================================================
   QUATERNIONS
   ============================================================ */

mat4 QuaternionToMat4(float qx, float qy, float qz, float qw)
{
    float x2 = qx + qx;
    float y2 = qy + qy;
    float z2 = qz + qz;

    float xx = qx * x2;
float yy = qy * y2;
    float zz = qz * z2;
    float xy = qx * y2;
    float xz = qx * z2;
    float yz = qy * z2;
    float wx = qw * x2;
    float wy = qw * y2;
    float wz = qw * z2;

    mat4 m = Mat4Identity();

    m.m[0][0] = 1.0f - (yy + zz);
    m.m[0][1] = xy + wz;
    m.m[0][2] = xz - wy;

    m.m[1][0] = xy - wz;
    m.m[1][1] = 1.0f - (xx + zz);
    m.m[1][2] = yz + wx;

    m.m[2][0] = xz + wy;
    m.m[2][1] = yz - wx;
    m.m[2][2] = 1.0f - (xx + yy);

    return m;
}


Vector4 QuaternionMultQuaternion(Vector4 a, Vector4 b)
{
    return (Vector4){
        a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z,
        a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
        a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
        a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w
    };
}


bool RayPlaneIntersection(ray_t* ray, plane_t* plane, vec_t* t_out){
  // Find t where ray origin + (ray dir * t) dot with normal = d
  
  float dNO = VectorDot(plane->normal, ray->origin);
  float dND = VectorDot(plane->normal, ray->dir);
  
  if (fabsf(dND) < EPSILON){
    // Handle near parallel case
    return false;
  }

  float t  = (plane->dist - dNO) / dND;
  
  if (t < 0.0){
    return false;
  }

  *t_out = t;
  return true;
}

bool RayPlaneIntersectionInBounds(ray_t* ray, plane_t* plane, vec_t halfs[2], vec_t* t_out){
  Vector u, v, helper;

  float dot_y = VectorDot(plane->normal, VECTOR_AXIS_Y);

  if (fabsf(dot_y) < 0.9f){
    helper = VECTOR_AXIS_Y;
  }else{
    helper = VECTOR_AXIS_X;
  }

  u = VectorCrossNormalise(helper, plane->normal);
  v = VectorCrossNormalise(plane->normal, u);

  float denom = VectorDot(ray->dir, plane->normal);
  if (fabsf(denom) < EPSILON)
    return false;
  
  Vector sub = VectorSub(
    VectorScale(plane->normal, plane->dist),
    ray->origin
  );
  
  float t = VectorDot(sub, plane->normal) / denom;
  if (t < 0.0f)
    return false;

  Vector intersection = VectorScale(ray->dir, t);
  intersection = VectorAdd(intersection, ray->origin);
  Vector plane_centre = VectorScale(plane->normal, plane->dist);
  Vector delta = VectorSub(intersection, plane_centre);

  vec_t du = VectorDot(delta, u);
  vec_t dv = VectorDot(delta, v);

  *t_out = t;
  return fabsf(du) <= halfs[0] && fabsf(dv) <= halfs[1];
}