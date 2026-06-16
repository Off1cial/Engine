#include "rendering/renderer.h"


void Camera_Switch(int index, int winw, int winh)
{
  if (index > MAX_CAMERAS)
  {
    return;
  }

  gCameraIndex = index;
  camera_t *cam = gCameras[index];
  gRendererState->active_cam = cam;
    // Convert top-left origin to bottom-left for OpenGL
    glViewport(
        cam->viewport.x,
        winh - cam->viewport.y - cam->viewport.h,
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

void R_DrawTextBatches(int winw, int winh)
{
  shader_t* shader = gRendererState->shader_unlit;
  Shader_Use(shader);
  gRendererState->shader_current = shader;

  // 2D batches — use orthographic projection matching screen pixels
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  // Get window size for ortho projection
  int winW = winw;
  int winH = winh;
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  for (size_t i = 0; i < gRendererState->textbatches2d_count; i++) {
    text_batch_t* batch = &gRendererState->textbatches2d[i];
    if (batch->mesh->vertex_count == 0) continue;

    MeshUpload(batch->mesh, GL_DYNAMIC_DRAW);
    glActiveTexture(GL_TEXTURE0);
    // Set orthographic projection (top-left origin, pixel coordinates)
    mat4 proj = Mat4Ortho(0.0f, (float)winW, (float)winH, 0.0f, -1.0f, 1.0f);
    mat4 view = Mat4Identity();
    mat4 model = Mat4Identity();
    Shader_SetVec4Cached(shader->uColourLoc, (Vector4){1, 1, 1, 1});
    Shader_SetMat4Cached(shader->uProjLoc, proj);
    Shader_SetMat4Cached(shader->uViewLoc, view);
    Shader_SetMat4Cached(shader->uModelLoc, model);
    Shader_SetIntCached(shader->uUseTextureLoc, 1);
    Shader_SetIntCached(shader->uUseVertexColLoc, 1);
    Shader_SetIntCached(shader->uTextureLoc, 0);
    glBindTexture(GL_TEXTURE_2D, batch->font->texture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    MeshDraw(batch->mesh, GL_TRIANGLES);
    glDisable(GL_BLEND);

    MeshReset(batch->mesh);
  }

  // 3D batches — use camera's projection
  glEnable(GL_DEPTH_TEST);

  for (size_t i = 0; i < gRendererState->textbatches3d_count; i++) {
    text_batch_t* batch = &gRendererState->textbatches3d[i];
    if (batch->mesh->vertex_count == 0) continue;

    MeshUpload(batch->mesh, GL_DYNAMIC_DRAW);

    camera_t* cam = gRendererState->active_cam;
    Shader_SetMat4Cached(shader->uProjLoc, cam->projection);
    Shader_SetMat4Cached(shader->uViewLoc, cam->view);
    Shader_SetMat4Cached(shader->uModelLoc, Mat4Identity());
    Shader_SetIntCached(shader->uUseTextureLoc, 1);
    Shader_SetIntCached(shader->uUseVertexColLoc, 1);

    glBindTexture(GL_TEXTURE_2D, batch->font->texture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    MeshDraw(batch->mesh, GL_TRIANGLES);
    glDisable(GL_BLEND);

    MeshReset(batch->mesh);
  }
  glEnable(GL_CULL_FACE);
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

  for (int t = 0; t < renderer->textbatches2d_count; t++)
  {
    text_batch_t* batch = &renderer->textbatches2d[t];
    Font_Destroy(batch->font);
    batch->screen_space = 0;
    batch->mesh_init = 0;
    MeshDestroy(batch->mesh);
  }
  free(renderer->materials);
  size_t light_count = 0;
  size_t light_forward_count = 0;
  RDrawQueue_Destroy(renderer->draw_q);
  renderer->draw_q = NULL;
  renderer->fullbright = false;
  renderer->wireframe = false;

}


