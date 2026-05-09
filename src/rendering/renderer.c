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
  if (material && material->shader){
    return material->shader;
  }

  return gRendererState->shader_unlit;
}