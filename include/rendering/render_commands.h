#ifndef RENDERERING_RENDERCOMMANDS_H
#define RENDERERING_RENDERCOMMANDS_H

#include "rendering/mesh.h"
#include "rendering/shader.h"
#include "rendering/material.h"
#include "rendering/text.h"

enum rcmd_type{
  RCMD_DRAW_MESH,
};

struct rcmd_t{
  enum rcmd_type type;
  union{
    struct{
      mesh_t* mesh;
      mat4 model;
      GLenum mode; // lines or triangles
      material_t* material;
      bool use_colour_override;
      Vector4 colour_override;
      bool wireframe;
    }draw_mesh;
  };
};

void Text_AddString(font_t *font, const char *text, Vector pos, Vector4 colour, bool screen_space);

struct rcmd_t* R_CreateRCMD(mesh_t* mesh, mat4 model, GLenum mode, material_t* material, bool use_col_override, Vector4 col_override, bool force_wireframe);

extern mesh_t* BSP_DEBUG_SPLITPLANES_MESHES[4096];
extern size_t BSP_DEBUG_SPLITPLANES_COUNT;
void R_DrawMesh(struct rcmd_t* cmd);

void R_DrawAABB(Vector centre, Vector halfs, Vector4 colour);


#endif
