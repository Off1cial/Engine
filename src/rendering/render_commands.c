#include "rendering/render_commands.h"

shader_t* SHADER_current_shader = NULL;
shader_t* SHADER_default_shader = NULL;
shader_t* SHADER_default_shader_lit = NULL;
shader_t* SHADER_default_shader_ui = NULL;
shader_t* SHADER_default_shader_billboard = NULL;

void RCMD_DrawMesh(struct rcmd_t cmd){
  shader_t* shader_active = cmd.draw_mesh.shader;

  if (shader_active != SHADER_current_shader){
    // swap shader or ordering failed
    Shader_Use(shader_active);
    SHADER_current_shader = shader_active;
  }


  Shader_SetMat4(shader_active, "uView", cmd.draw_mesh.view);
  Shader_SetMat4(shader_active, "uProj", cmd.draw_mesh.projection);
  Shader_SetMat4(shader_active, "uModel", cmd.draw_mesh.model);

  MeshDraw(cmd.draw_mesh.mesh, cmd.draw_mesh.mode);
}
