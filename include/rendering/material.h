#ifndef MATERIAL_H
#define MATERIAL_H

#include "rendering/texture.h"
#include "rendering/shader.h"
#include "types/types_vector.h"

#define MAX_MATERIALS 128

typedef enum material_flags_t{
  MATERIAL_USE_TEXTURE      = 1 << 0,
  MATERIAL_USE_VERTEX_COLOUR= 1 << 1,
  MATERIAL_TRANSPARENT      = 1 << 2,
  MATERIAL_DOUBLE_SIDED     = 1 << 3,
  MATERIAL_UNLIT            = 1 << 4,
  MATERIAL_SPECULAR         = 1 << 5,

} material_flags_t;


typedef struct {
  shader_t* shader; // NULL = use default lit or unlit shader

  texture_t* base;


  float specular;

  Vector4 colour;
  uint32_t flags;

} material_t;


//bool Material_HasFlag(material_t* material, material_flags_t flag);
static inline bool Material_HasFlag(const material_t* material, material_flags_t flag){
  if (!material){return false;}

  return (material->flags & flag) != 0;
}


material_t* Material_Load(const char* filepath);

#endif
