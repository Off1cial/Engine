#ifndef RENDERER_H
#define RENDERER_H

#include "rendering/draw_list.h"
#include "rendering/material.h"
#include "rendering/camera.h"
#include "rendering/light.h"


typedef struct {
  camera_t* active_cam;
  material_t* materials;

  shader_store_t* shader_store;
  shader_t* shader_lit;
  shader_t* shader_unlit;
  shader_t* shader_current;

  light_t lights[MAX_LIGHTS];
  size_t light_count;

  size_t material_count;
  rdrawqueue_t* draw_q;

  bool fullbright;
  bool wireframe;

} renderer_state_t;

void Camera_Switch(int index, int winh);

extern renderer_state_t* gRendererState;


shader_t* Renderer_ResolveShaderFromMaterial(material_t* material);

#endif