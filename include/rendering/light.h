#ifndef LIGHT_H
#define LIGHT_H

#include "types/types_vector.h"

#define MAX_LIGHTS 128


typedef enum light_type_t{
    LIGHT_DIRECTIONAL = 0,
    LIGHT_POINT = 1,
    LIGHT_SPOT = 2
} light_type_t;


typedef struct light_t{
  light_type_t type;

  Vector position;
  Vector direction;

  Vector colour;

  float intensity;
  float radius;
  float cutoff;
} light_t;

#endif