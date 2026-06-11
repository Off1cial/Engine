#include "rendering/render_commands.h"
#include "rendering/renderer.h"
#include "rendering/camera.h"

#include "tools/flagtool.h"
#include "mem.h"

struct rcmd_t *R_CreateRCMD_Mesh(mesh_t *mesh, mat4 model, GLenum mode, material_t *material, bool use_col_override, Vector4 col_override, bool force_wireframe);

void Text_AddString(font_t *font, const char *text, Vector pos, Vector4 colour, bool screen_space)
{
  text_batch_t *batch = NULL;
  text_batch_t *batches = (screen_space) ? gRendererState->textbatches2d : gRendererState->textbatches3d;
  size_t *count = (screen_space) ? &gRendererState->textbatches2d_count : &gRendererState->textbatches3d_count;
  // Find batch for this font and rendering space
  for (size_t i = 0; i < *count; i++)
  {
    if (batches[i].font == font)
    {
      batch = &batches[i];
      break;
    }
  }
  // No batch found
  if (!batch && *count < RENDERER_MAX_TEXTBACTHES)
  {
    
    batch = &batches[(*count)++];
    batch->font = font;
    batch->screen_space = screen_space;
    batch->quad_count = 0;
    if (!batch->mesh_init)MeshInit(&batch->mesh, 4096 * 4, 4096 * 6); // pre-allocate
    batch->mesh_init = true;
    printf("[FONTS]: Font batch created, batch count = %zu\n", *count);
  }

  float cursorX = pos.x;
  float cursorY = pos.y;

  for (const char *c = text; *c; c++)
  {
    unsigned char idx = (unsigned char)*c;
    if (idx < 32 || idx >= MAX_GLYPHS)
    {
      cursorX += font->glyphs[' '].advance;
      continue;
    }

    glyph_t *g = &font->glyphs[idx];
    if (g->w == 0 && g->h == 0)
    {
      cursorX += font->glyphs[' '].advance;
      continue;
    }

    float x0 = cursorX + (float)g->bearingX;
    float y0 = cursorY - (float)(g->h - g->bearingY);
    float x1 = x0 + (float)g->w;
    float y1 = y0 + (float)g->h;

    Vector col = {colour.x, colour.y, colour.z};
    struct vertex_t v0 = {.pos = {x0, y0, 0}, .colour = col, .uv = {g->u0, g->v0}, .normal = VECTOR_AXIS_Z, .tangent = VECTOR_AXIS_X};
    struct vertex_t v1 = {.pos = {x1, y0, 0}, .colour = col, .uv = {g->u1, g->v0}, .normal = VECTOR_AXIS_Z, .tangent = VECTOR_AXIS_X};
    struct vertex_t v2 = {.pos = {x1, y1, 0}, .colour = col, .uv = {g->u1, g->v1}, .normal = VECTOR_AXIS_Z, .tangent = VECTOR_AXIS_X};
    struct vertex_t v3 = {.pos = {x0, y1, 0}, .colour = col, .uv = {g->u0, g->v1}, .normal = VECTOR_AXIS_Z, .tangent = VECTOR_AXIS_X};

    GLuint i0 = MeshPushVertex(&batch->mesh, v0);
    GLuint i1 = MeshPushVertex(&batch->mesh, v1);
    GLuint i2 = MeshPushVertex(&batch->mesh, v2);
    GLuint i3 = MeshPushVertex(&batch->mesh, v3);

    MeshPushTriangle(&batch->mesh, i0, i1, i2);
    MeshPushTriangle(&batch->mesh, i0, i2, i3);

    cursorX += (float)g->advance;
  }
  MeshUpload(&batch->mesh, GL_DYNAMIC_DRAW);
}



static void Renderer_BindMaterial(shader_t *shader, material_t *material)
{
  if (!material)
  {
    return;
  }

  Vector4 colour;
  if (RENDERER_HASFLAG(gRendererState, RENDERER_FLAG_WIREFRAME))
  {
    colour.x = 0.0f;
    colour.y = 1.0f;
    colour.z = 1.0f;
    colour.w = 1.0f;
  }
  else
  {
    colour = material->colour;
  }

  // Base colour
  Shader_SetVec4(
      shader,
      "uColour",
      colour);

  // Handle lights
  if (!Material_HasFlag(material, MATERIAL_UNLIT) && !RENDERER_HASFLAG(gRendererState, RENDERER_FLAG_FULLBRIGHT))
  {
    Shader_SetIntCached(shader->uLightCountLoc, gRendererState->light_forward_count);

    // Sumbit lights
    for (size_t i = 0; i < gRendererState->light_forward_count; i++)
    {
      light_t *light = gRendererState->lights_forward[i];

      if (!light)
        continue;

      if (shader->uLights[i].position != -1)
        glUniform3f(shader->uLights[i].position, light->position.x, light->position.y, light->position.z);
      if (shader->uLights[i].direction != -1)
        glUniform3f(shader->uLights[i].direction, light->direction.x, light->direction.y, light->direction.z);
      if (shader->uLights[i].colour != -1)
        glUniform3f(shader->uLights[i].colour, light->colour.x, light->colour.y, light->colour.z);

      if (shader->uLights[i].intensity != -1)
        glUniform1f(shader->uLights[i].intensity, light->intensity);
      if (shader->uLights[i].radius != -1)
        glUniform1f(shader->uLights[i].radius, light->radius);
      if (shader->uLights[i].cutoff != -1)
        glUniform1f(shader->uLights[i].cutoff, light->cutoff);
      if (shader->uLights[i].type != -1)
        glUniform1i(shader->uLights[i].type, light->type);
    }

    
  }

  bool use_texture =
      Material_HasFlag(material, MATERIAL_USE_TEXTURE) &&
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
      use_texture);
  Shader_SetIntCached(
      shader->uUseVertexColLoc,
      use_vertex_col);
  if (use_texture)
  {
    Texture_Bind(material->base, 0);
    Shader_SetIntCached(shader->uTextureLoc, 0);
  }

  // Normal
  if (use_normals)
  {
    Shader_SetIntCached(shader->uUseNormalMapLoc, gRendererState->draw_normal_maps);
    Texture_Bind(material->normal, 1);
    Shader_SetIntCached(shader->uNormalMapLoc, 1);
  }
  else
  {
    Shader_SetIntCached(shader->uUseNormalMapLoc, 0);
  }

  // Transparency
  if ((material->flags & MATERIAL_TRANSPARENT) != 0)
  {
    glEnable(GL_BLEND);
    glBlendFunc(
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA);
  }
  else
  {
    glDisable(GL_BLEND);
  }

  // Specular/Shininess

  if (((material->flags & MATERIAL_SPECULAR) != 0))
  {
    Shader_SetFloatCached(shader->uSpecularLoc, material->specular);
    Shader_SetFloatCached(shader->uShininessLoc, material->shininess);
  }
  else
  {
    Shader_SetFloatCached(shader->uSpecularLoc, 0.0f);
    Shader_SetFloatCached(shader->uShininessLoc, 0.0f);
  }

  // Double sided
  if ((material->flags & MATERIAL_DOUBLE_SIDED) != 0)
  {
    glDisable(GL_CULL_FACE);
  }
  else
  {
    glEnable(GL_CULL_FACE);
  }
}

void R_DrawMesh(struct rcmd_t *cmd)
{

  if (!cmd->draw_mesh.mesh)
  {
    printf("Mesh is NULL\n");
    exit(1);
  }
  if (cmd->draw_mesh.mesh->nan)
  {
    printf("Nan mesh\n");
    return;
  }

  bool fullbright =
      RENDERER_HASFLAG(gRendererState, RENDERER_FLAG_FULLBRIGHT) ||
      RENDERER_HASFLAG(gRendererState, RENDERER_FLAG_WIREFRAME);

  shader_t *shader_active = NULL;
  if (fullbright)
  {
    shader_active = gRendererState->shader_unlit;
  }
  else
  {
    shader_active = Renderer_ResolveShaderFromMaterial(cmd->draw_mesh.material);
  }

  if ((shader_active != gRendererState->shader_current) && (shader_active != NULL))
  {
    // swap shader or ordering failed
    // printf("Shader swapped\n");
    Shader_Use(shader_active);
    gRendererState->shader_current = shader_active;
  }

  Renderer_BindMaterial(shader_active, cmd->draw_mesh.material);
  // Override colour if requested
  if (cmd->draw_mesh.use_colour_override)
  {
    Shader_SetVec4Cached(shader_active->uColourLoc, cmd->draw_mesh.colour_override);
    Shader_SetIntCached(shader_active->uUseTextureLoc, 0);
  }

  Shader_SetVec3Cached(shader_active->uViewPosLoc, gRendererState->active_cam->pos);
  Shader_SetMat4Cached(shader_active->uViewLoc, gRendererState->active_cam->view);
  Shader_SetMat4Cached(shader_active->uProjLoc, gRendererState->active_cam->projection);
  Shader_SetMat4Cached(shader_active->uModelLoc, cmd->draw_mesh.model);

  if (RENDERER_HASFLAG(gRendererState, RENDERER_FLAG_WIREFRAME) || cmd->draw_mesh.wireframe)
  {
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }
  else
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  MeshDraw(cmd->draw_mesh.mesh, GL_TRIANGLES);
}

void R_DrawAABB(Vector centre, Vector halfs, Vector4 colour)
{
  //mat4 model = Mat4Identity();

  mat4 T = Mat4Identity();
  T.m[3][0] = centre.x;
  T.m[3][1] = centre.y;
  T.m[3][2] = centre.z;


  mat4 S = Mat4Identity();
  S.m[0][0] = halfs.x * 2.0f;
  S.m[1][1] = halfs.y * 2.0f;
  S.m[2][2] = halfs.z * 2.0f;
  mat4 model = Mat4Mul(&T, &S);

  // printf("[RENDERER]: Drawing AABB\n  Centre: {%0.2f, %0.2f, %0.2f}\n  Halfs: {%0.2f, %0.2f, %0.2f}\n", centre.x, centre.y, centre.z, halfs.x, halfs.y, halfs.z);

  struct rcmd_t *rcmd = MEM_ARENA_ALLOC(gMemArena, sizeof(struct rcmd_t), alignof(struct rcmd_t));
  rcmd->type = RCMD_DRAW_MESH;
  rcmd->draw_mesh.mode = GL_DYNAMIC_DRAW;
  rcmd->draw_mesh.material = gRendererState->materials[1];
  rcmd->draw_mesh.mesh = MESH_PRIMITIVES[MESH_PRIMITIVE_CUBE];
  rcmd->draw_mesh.model = model;
  rcmd->draw_mesh.wireframe = true;
  rcmd->draw_mesh.colour_override = colour;
  rcmd->draw_mesh.use_colour_override = true;

  RDrawQueue_Push(gRendererState->draw_q, rcmd);
}