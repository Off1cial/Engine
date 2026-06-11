#ifndef VTYPES_H
#define VTYPES_H

#include "types_base.h"
#include <math.h>
#include "math/mathlib.h"

#define FLOAT32_NAN_BITS     (unsigned long)0x7FC00000	// not a number!
#define FLOAT32_NAN          BitsToFloat( FLOAT32_NAN_BITS )
#define VEC_T_NAN FLOAT32_NAN

#define VectorNew() (VectorZero())
#define VECTOR_ZERO ((Vector){0, 0, 0})
#define VECTOR_ONE ((Vector){1, 1, 1})
#define VECTOR_NAN ((Vector){VEC_T_NAN, VEC_T_NAN, VEC_T_NAN})

#define VECTOR_IS_NAN(v) (v.x == VEC_T_NAN || v.x == NAN || \
                          v.y == VEC_T_NAN || v.y == NAN|| \
                          v.z == VEC_T_NAN || v.z == NAN)


                          
#define VECTOR_IS_ZERO(v) (v.x == 0.0f || \
                           v.y == 0.0f || \
                           v.z == 0.0f)

#define VECTOR_AXIS_X ((Vector){1, 0, 0 })
#define VECTOR_AXIS_Y ((Vector){0, 1, 0 })
#define VECTOR_AXIS_Z ((Vector){0, 0, 1 })

#define VECTOR_AXIS_X_NEG ((Vector){-1, 0, 0 })
#define VECTOR_AXIS_Y_NEG ((Vector){0, -1, 0 })
#define VECTOR_AXIS_Z_NEG ((Vector){0, 0, -1 })

#define EPSILON 1e-6f

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


typedef struct {
  Vector origin;
  Vector dir;
} ray_t;


typedef struct {
  Vector normal;
  vec_t dist;
} plane_t;
#ifdef __cplusplus
extern "C" {
#endif

void Mat4Copy(const mat4 src, mat4* dest);
mat4 Mat4Mul(mat4* a, mat4* b);
void Mat4MulTo(mat4* a, mat4* b, mat4* dest);
Vector4 Mat4Mulv(mat4* m, Vector4* v);
mat4 Mat4Translate(Vector* t);
mat4 Mat4Scale(Vector* s);
mat4 Mat4Rotate(float angle, Vector axis);
mat4 Mat4LookAt(Vector eye, Vector center, Vector up);
mat4 Mat4Ortho(float left, float right, float bottom, float top, float near, float far);
mat4 Mat4Perspective(float fovY, float aspect, float znear, float zfar);
bool Mat4Inverse(mat4* m, mat4* out);
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

Vector VectorNegate(Vector a);
void VectorNegateInPlace(Vector* a);
void VectorNegateTo(Vector a, Vector* dest);

Vector VectorNormalise(Vector a);

int VectorCompare_Imprecise(Vector a, Vector b, float min, float max);
int VectorEqual(Vector a, Vector b);

void VectorCopy(Vector a, Vector* dest);
Vector VectorZero();


float Vector2Cross(Vector2 o, Vector2 a, Vector2 b);
Vector2 Vector2Add(Vector2 a, Vector2 b);
Vector2 Vector2Scale(Vector2 v, float scale);


plane_t PlaneReverse(plane_t plane);


// Extras



bool RayPlaneIntersection(ray_t* ray, plane_t* plane, vec_t* t_out);
bool RayPlaneIntersectionInBounds(ray_t* ray, plane_t* plane, vec_t halfs[2], vec_t* t_out);


// Debug
void Vector_DPrint(Vector* a);





#ifdef __cplusplus
}
#endif

#endif
