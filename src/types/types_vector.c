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
    (Vector){-1, 0, 0},
    VECTOR_AXIS_Y,
    (Vector){0, -1, 0},
    VECTOR_AXIS_Z,
    (Vector){0, 0, -1}
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
    Vector f = VectorSub(center, eye);
    VectorNormalise(&f);

    Vector r = VectorCross(f, up);
    VectorNormalise(&r);

    Vector u = VectorCross(r, f);

    mat4 view = Mat4Identity();

    view.m[0][0] = r.x;
    view.m[0][1] = u.x;
    view.m[0][2] = -f.x;

    view.m[1][0] = r.y;
    view.m[1][1] = u.y;
    view.m[1][2] = -f.y;

    view.m[2][0] = r.z;
    view.m[2][1] = u.z;
    view.m[2][2] = -f.z;

    view.m[3][0] = -VectorDot(r, eye);
    view.m[3][1] = -VectorDot(u, eye);
    view.m[3][2] =  VectorDot(f, eye);

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
