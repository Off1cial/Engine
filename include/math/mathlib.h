#ifndef MATH_MATHLIB_H
#define MATH_MATHLIB_H

#include <math.h>
#ifndef M_PI
# define M_PI 3.14159265358979323846 
#endif

#define RAD(x) (x * M_PI/180.0f)
#define DEG(X) (x * (180.0f/M_PI))

#endif
