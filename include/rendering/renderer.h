#ifndef RENDERER_H
#define RENDERER_H

#include "rendering/draw_list.h"
#include "rendering/material.h"
#include "rendering/camera.h"
#include "rendering/light.h"

typedef enum renderer_flags_t{
  RENDERER_FLAG_NONE  = 0,
  RENDERER_FLAG_FULLBRIGHT     = 1 << 0,
  RENDERER_FLAG_FLATTEXTURE    = 1 << 1,
  RENDERER_FLAG_WIREFRAME      = 1 << 2,
} renderer_flags_t;

#define RENDERER_HASFLAG(r, f) ( ((r)->flags & f) != 0)

typedef struct {
  camera_t* active_cam;
  

  shader_store_t* shader_store;
  shader_t* shader_lit;
  shader_t* shader_unlit;
  shader_t* shader_current;

  light_t lights[MAX_LIGHTS];
  size_t light_count;

  light_t* lights_forward[MAX_FORWARD_LIGHTS];
  size_t light_forward_count;

  material_t** materials;
  size_t material_count;
  rdrawqueue_t* draw_q;

  
  bool draw_normal_maps;

  uint32_t flags;

  bool fullbright;
  bool wireframe;

} renderer_state_t;

void Camera_Switch(int index, int winh);

extern renderer_state_t* gRendererState;

void Renderer_AddMaterial(renderer_state_t* renderer, material_t* material);
shader_t* Renderer_ResolveShaderFromMaterial(material_t* material);

void Renderer_Destroy(renderer_state_t* renderer);

#endif
