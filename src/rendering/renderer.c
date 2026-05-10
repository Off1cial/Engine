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


static void destroy_material(material_t* m){
  if (!m){
    return;
  }

  if (m->shader){
    Shader_Destroy(m->shader); 
    m->shader = NULL;
  }
  m->base = NULL;
  m->specular = 0;

  free(m);
}

void Renderer_Destroy(renderer_state_t* renderer){
  ShaderStore_Free(renderer->shader_store);
  
  for (size_t m = 0; m < renderer->material_count; m++){
    destroy_material(renderer->materials[m]); 
  }
  free(renderer->materials);
  size_t light_count = 0;
  size_t light_forward_count = 0;
  RDrawQueue_Destroy(renderer->draw_q);
  renderer->draw_q = NULL;
  renderer->fullbright = false;
  renderer->wireframe = false;

}
