#include "rendering/render_commands.h"
#include "rendering/renderer.h"
#include "rendering/camera.h"


static void Renderer_BindMaterial(shader_t* shader, material_t* material){
  if (!material){ return; }

  Vector4 colour;
  if (RENDERER_HASFLAG(gRendererState, RENDERER_FLAG_WIREFRAME)){
    colour.x = 0.0f;
    colour.y = 1.0f;
    colour.z = 1.0f;
    colour.w = 1.0f;
  }else{
    colour = material->colour;
  }

  // Base colour
  Shader_SetVec4(
    shader,
    "uColour",
    colour
  );

  // Handle lights
  if (!Material_HasFlag(material, MATERIAL_UNLIT) && !RENDERER_HASFLAG(gRendererState, RENDERER_FLAG_FULLBRIGHT)){
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

  bool use_texture = 
    Material_HasFlag(material, MATERIAL_USE_TEXTURE)           && 
    !RENDERER_HASFLAG(gRendererState, RENDERER_FLAG_WIREFRAME) && 
    material->base;

  bool use_vertex_col = 
    Material_HasFlag(material, MATERIAL_USE_VERTEX_COLOUR) && 
    !RENDERER_HASFLAG(gRendererState, RENDERER_FLAG_WIREFRAME);

  bool use_normals = 
    Material_HasFlag(material, MATERIAL_USE_NORMAL) && 
    !RENDERER_HASFLAG(gRendererState, RENDERER_FLAG_FLATTEXTURE) &&
    material->normal;

  //  TEXTURE
  Shader_SetIntCached(
    shader->uUseTextureLoc,
    use_texture
  );
  Shader_SetIntCached(
    shader->uUseVertexColLoc,
    use_vertex_col
  );
  if (use_texture){
    Texture_Bind(material->base, 0);
    Shader_SetIntCached(shader->uTextureLoc, 0);
  }




  // Normal
  if (use_normals){
    Shader_SetIntCached(shader->uUseNormalMapLoc, gRendererState->draw_normal_maps);
    Texture_Bind(material->normal, 1);
    Shader_SetIntCached(shader->uNormalMapLoc, 1);
  }else{
    Shader_SetIntCached(shader->uUseNormalMapLoc, 0);
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

  // Specular/Shininess

  if (((material->flags & MATERIAL_SPECULAR) != 0)){
    Shader_SetFloatCached(shader->uSpecularLoc, material->specular);
    Shader_SetFloatCached(shader->uShininessLoc, material->shininess);
  }else{
    Shader_SetFloatCached(shader->uSpecularLoc, 0.0f);
    Shader_SetFloatCached(shader->uShininessLoc, 0.0f);
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

  bool fullbright = 
    RENDERER_HASFLAG(gRendererState, RENDERER_FLAG_FULLBRIGHT) || 
    RENDERER_HASFLAG(gRendererState, RENDERER_FLAG_WIREFRAME);

  shader_t* shader_active = NULL;
  if (fullbright){
    shader_active = gRendererState->shader_unlit;
  }else{
    shader_active = Renderer_ResolveShaderFromMaterial(cmd->draw_mesh.material);
  }


  

  


  if ((shader_active != gRendererState->shader_current) && (shader_active != NULL)){
    // swap shader or ordering failed
    //printf("Shader swapped\n");
    Shader_Use(shader_active);
    gRendererState->shader_current = shader_active;
  }

  Renderer_BindMaterial(shader_active, cmd->draw_mesh.material);


  Shader_SetVec3Cached(shader_active->uViewPosLoc, gRendererState->active_cam->pos);
  Shader_SetMat4Cached(shader_active->uViewLoc, gRendererState->active_cam->view);
  Shader_SetMat4Cached(shader_active->uProjLoc, gRendererState->active_cam->projection);
  Shader_SetMat4Cached(shader_active->uModelLoc, cmd->draw_mesh.model);

  if (RENDERER_HASFLAG(gRendererState, RENDERER_FLAG_WIREFRAME)) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  MeshDraw(cmd->draw_mesh.mesh, GL_TRIANGLES);
}
