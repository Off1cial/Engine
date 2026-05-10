#include "rendering/renderer.h"


void Camera_Switch(int index, int winh)
{
  if (index > MAX_CAMERAS)
  {
    return;
  }

  gCameraIndex = index;
  camera_t *cam = gCameras[index];
  gRendererState->active_cam = cam;
  glViewport(
      cam->viewport.x,
      winh - cam->viewport.y,
      cam->viewport.w,
      cam->viewport.h);
}

shader_t* Renderer_ResolveShaderFromMaterial(material_t* material){
  if (!material){
    return gRendererState->shader_unlit;
  }

  if (Material_HasFlag(material, MATERIAL_UNLIT)){
    return gRendererState->shader_unlit;
  }

  return gRendererState->shader_lit;
}


void Renderer_AddMaterial(renderer_state_t* renderer, material_t* material){

  if (!renderer->materials && renderer->material_count == 0){
    renderer->materials = malloc(sizeof(material_t*) * MAX_MATERIALS);
  }
  if (!renderer || !material){ return; }

  if (renderer->material_count >= MAX_MATERIALS){
    return;
  }
  renderer->materials[renderer->material_count++] = material;
}
