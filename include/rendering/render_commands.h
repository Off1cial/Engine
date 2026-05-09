#ifndef RENDERERING_RENDERCOMMANDS_H
#define RENDERERING_RENDERCOMMANDS_H

#include "rendering/mesh.h"
#include "rendering/shader.h"
#include "rendering/material.h"

enum rcmd_type{
  RCMD_DRAW_MESH,
};

struct rcmd_t{
  enum rcmd_type type;
  union{
    struct{
      mesh_t* mesh;
      //shader_t* shader;
      mat4 model;
      //mat4 view;
      //mat4 projection;
      GLenum mode;
      material_t* material;
    }draw_mesh;
  };
};



void RCMD_DrawMesh(struct rcmd_t* cmd);


#endif
