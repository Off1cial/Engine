#ifndef LIGHT_H
#define LIGHT_H

#include "types/types_vector.h"

#define MAX_LIGHTS 128
#define MAX_FORWARD_LIGHTS 8

typedef enum light_type_t{
    LIGHT_DIRECTIONAL,
    LIGHT_POINT,
} light_type_t;


typedef struct light_t{
    light_type_t type;

    Vector position;
    Vector direction;

    Vector colour;

    float intensity;
    float radius;
} light_t;

#endif