#include "rendering/render_commands.h"
#include "rendering/renderer.h"
#include "rendering/camera.h"


static void Renderer_BindMaterial(shader_t* shader, material_t* material){
  if (!material){ return; }



  // Base colour
  Shader_SetVec4(
    shader,
    "uColour",
    material->colour
  );

  // Handle lights
  if (!Material_HasFlag(material, MATERIAL_UNLIT)){
    Shader_SetIntCached(shader->uLightCountLoc, gRendererState->light_forward_count);


    // Sumbit lights
    for (size_t i = 0; i < gRendererState->light_forward_count; i++){
      light_t* light = gRendererState->lights_forward[i];

      if (!light)
        continue;

      if (shader->uLights[i].position != -1)  glUniform3f(shader->uLights[i].position, light->position.x, light->position.y, light->position.z);
      if (shader->uLights[i].direction != -1) glUniform3f(shader->uLights[i].direction, light->direction.x, light->direction.y, light->direction.z);
      if (shader->uLights[i].colour != -1)    glUniform3f(shader->uLights[i].colour, light->colour.x, light->colour.y, light->colour.z);

      if (shader->uLights[i].intensity != -1) glUniform1f(shader->uLights[i].intensity, light->intensity);
      if (shader->uLights[i].radius != -1)    glUniform1f(shader->uLights[i].radius, light->radius);
      if (shader->uLights[i].type != -1)      glUniform1i(shader->uLights[i].type, light->type);

    }
  }


  // Handle Flags
  if (!(material->flags & MATERIAL_USE_TEXTURE) || !material->base){
    Shader_SetIntCached(shader->uUseTextureLoc, 0);
  }else{
    Shader_SetIntCached(
      shader->uUseTextureLoc,
      (material->flags & MATERIAL_USE_TEXTURE) != 0
    );
  }

  Shader_SetIntCached(
    shader->uUseVertexColLoc,
    (material->flags & MATERIAL_USE_VERTEX_COLOUR) != 0
  );
  // Set texture
  if ((material->flags & MATERIAL_USE_TEXTURE) != 0){
    if (material->base){
      Texture_Bind(material->base, 0);
      Shader_SetIntCached(shader->uTextureLoc, 0);
    }
  }
  // Transparency
  if ((material->flags & MATERIAL_TRANSPARENT) != 0){
    glEnable(GL_BLEND);
    glBlendFunc(
      GL_SRC_ALPHA,
      GL_ONE_MINUS_SRC_ALPHA
    );
  }else{
    glDisable(GL_BLEND);
  }

  // Double sided
  if ((material->flags & MATERIAL_DOUBLE_SIDED) != 0){
    glDisable(GL_CULL_FACE);
  }else {
    glEnable(GL_CULL_FACE);
  }

}

void RCMD_DrawMesh(struct rcmd_t* cmd){
  
  if (!cmd->draw_mesh.mesh){
    printf("Mesh is NULL\n");
    exit(1);
  }
  if (cmd->draw_mesh.mesh->nan){
    return;
  }


  shader_t* shader_active = Renderer_ResolveShaderFromMaterial(cmd->draw_mesh.material);
  

  


  if (shader_active != gRendererState->shader_current && shader_active != NULL){
    // swap shader or ordering failed
    printf("Shader swapped\n");
    Shader_Use(shader_active);
    gRendererState->shader_current = shader_active;
  }

  Renderer_BindMaterial(shader_active, cmd->draw_mesh.material);

  Shader_SetMat4Cached(shader_active->uViewLoc, gRendererState->active_cam->view);
  Shader_SetMat4Cached(shader_active->uProjLoc, gRendererState->active_cam->projection);
  Shader_SetMat4Cached(shader_active->uModelLoc, cmd->draw_mesh.model);


  MeshDraw(cmd->draw_mesh.mesh, cmd->draw_mesh.mode);
}
