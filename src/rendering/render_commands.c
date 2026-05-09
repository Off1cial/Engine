#include "rendering/render_commands.h"
#include "rendering/renderer.h"
#include "rendering/camera.h"


void RCMD_DrawMesh(struct rcmd_t* cmd){

  shader_t* shader_active = Renderer_ResolveShaderFromMaterial(cmd->draw_mesh.material);
  if (!cmd->draw_mesh.mesh){
    printf("Mesh is NULL\n");
    exit(1);
  }

  if (shader_active != gRendererState->shader_current && shader_active != NULL){
    // swap shader or ordering failed
    printf("Shader swapped\n");
    Shader_Use(shader_active);
    gRendererState->shader_current = shader_active;
  }

  Shader_SetMat4(shader_active, "uView", gRendererState->active_cam->view);
  Shader_SetMat4(shader_active, "uProj", gRendererState->active_cam->projection);
  Shader_SetMat4(shader_active, "uModel", cmd->draw_mesh.model);
  
  MeshDraw(cmd->draw_mesh.mesh, cmd->draw_mesh.mode);
}
