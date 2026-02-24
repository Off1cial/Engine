#include "rendering/render_commands.h"
#include "rendering/camera.h"

shader_t* SHADER_current_shader = NULL;
shader_t* SHADER_default_shader = NULL;
shader_t* SHADER_default_shader_lit = NULL;
shader_t* SHADER_default_shader_ui = NULL;
shader_t* SHADER_default_shader_billboard = NULL;

void RCMD_DrawMesh(struct rcmd_t* cmd){
  printf("Executing\n");
  if (NULL == cmd->draw_mesh.mesh){
    printf("Mesh is null\n");
  }else{
    printf("USING MESH AT : %p\n", cmd->draw_mesh.mesh);
  }

  shader_t* shader_active = cmd->draw_mesh.shader;
  Shader_Use(cmd->draw_mesh.shader);
  
  /*
  if (shader_active != SHADER_current_shader){
    // swap shader or ordering failed
    printf("Shader swapped\n");
    Shader_Use(shader_active);
    SHADER_current_shader = shader_active;
  }
  */
  

  Shader_SetMat4(shader_active, "uView", gCameras[gCameraIndex]->view);
  Shader_SetMat4(shader_active, "uProj", gCameras[gCameraIndex]->projection);
  Shader_SetMat4(shader_active, "uModel", Mat4Identity());
  
  printf("Mesh verts: %zu\n", cmd->draw_mesh.mesh->vertex_count);
  MeshDraw(cmd->draw_mesh.mesh, cmd->draw_mesh.mode);
}
