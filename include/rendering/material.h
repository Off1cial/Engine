#ifndef MATERIAL_H
#define MATERIAL_H

#include "rendering/texture.h"
#include "rendering/shader.h"
#include "types/types_vector.h"



typedef enum material_flags_t{
    MATERIAL_USE_TEXTURE      = 1 << 0,
    MATERIAL_USE_VERTEX_COLOUR= 1 << 1,
    MATERIAL_TRANSPARENT      = 1 << 2,
    MATERIAL_DOUBLE_SIDED     = 1 << 3,
    MATERIAL_UNLIT            = 1 << 4,
} material_flags_t;


typedef struct {
  shader_t* shader; // NULL = use default lit or unlit shader

  texture_t* base;

  float colour[4];
  uint32_t flags;

} material_t;


#endif