#ifndef VTYPES_H
#define VTYPES_H

#include "types_base.h"
#include <math.h>

#define FLOAT32_NAN_BITS     (unsigned long)0x7FC00000	// not a number!
#define FLOAT32_NAN          BitsToFloat( FLOAT32_NAN_BITS )
#define VEC_T_NAN FLOAT32_NAN

#define VectorNew() (VectorZero())
#define VECTOR_ZERO ((Vector){0, 0, 0})
#define VECTOR_ONE ((Vector){1, 1, 1})

#define VECTOR_AXIS_X ((Vector){1, 0, 0 })
#define VECTOR_AXIS_Y ((Vector){0, 1, 0 })
#define VECTOR_AXIS_Z ((Vector){0, 0, 1 })

typedef enum {
  X_POS,
  X_NEG,
  Y_POS,
  Y_NEG,
  Z_POS,
  Z_NEG
} eAXIS;



typedef float vec_t;

typedef union vec3_t {
  struct{
    vec_t x,y,z;
  };
  vec_t v[3];
}Vector;

extern Vector VECTOR_AXES[6];


typedef union vec4_t{
  struct{
    vec_t x,y,z,w;
  };
  vec_t v[4];
}Vector4;

typedef union vec2_t{
  struct {
    vec_t x,y;
  };
  vec_t v[2];
}Vector2;

typedef union mat3_t{
  vec_t m[3][3];
} mat3;

static mat3 Mat3Identity(){
  return  
  (mat3){
  {1, 0, 0,
      0, 1, 0,
      0, 0, 1
    },
  };
}

typedef union mat4_t{
  vec_t m[4][4];
} mat4 __attribute__((aligned(16)));

static mat4 Mat4Identity(){
  return
  (mat4){
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  };
}
void Mat4Copy(const mat4 src, mat4* dest);
mat4 Mat4Mul(mat4* a, mat4* b);
void Mat4MulTo(mat4* a, mat4* b, mat4* dest);
mat4 Mat4Translate(Vector* t);
mat4 Mat4Scale(Vector* s);
mat4 Mat4Rotate(float angle, Vector axis);
mat4 Mat4LookAt(Vector eye, Vector center, Vector up);
mat4 Mat4Perspective(float fovY, float aspect, float znear, float zfar);
mat4 QuaternionToMat4(float qx, float qy, float qz, float qw);
Vector Mat4Transform(mat4* m, Vector* v);
Vector4 QuaternionMultQuaternion(Vector4 a, Vector4 b);


inline bool IsValid(Vector* v)
{
    return !(isnan(v->x) || isnan(v->y) || isnan(v->z));
}



Vector VectorInit(float x, float y, float z);

double VectorDot(Vector a, Vector b);
double VectorMag(Vector a);
double VectorMag2(Vector a);
double VectorAngle(Vector a, Vector b);

Vector VectorAdd(Vector a, Vector b);
Vector VectorSub(Vector a, Vector b);

Vector VectorScale(Vector a, float scale);
void VectorScaleInPlace(Vector* a, float scale);
void VectorScaleTo(Vector a, float scale, Vector* dest);

Vector VectorCross(Vector a, Vector b);
Vector VectorCrossNormalise(Vector a, Vector b);

void VectorNegate(Vector* a);
void VectorNegateTo(Vector a, Vector* dest);

void VectorNormalise(Vector* a);
void VectorNormaliseTo(Vector a, Vector* dest);

int VectorCompare_Imprecise(Vector a, Vector b, float min, float max);
int VectorEqual(Vector a, Vector b);

void VectorCopy(Vector a, Vector* dest);

Vector VectorZero();


// Debug
void Vector_DPrint(Vector* a);

#endif
