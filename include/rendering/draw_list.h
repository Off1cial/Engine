#ifndef RENDERERING_DRAW_LIST_H
#define RENDERERING_DRAW_LIST_H

#include "rendering/mesh.h"
#include "rendering/shader.h"

enum rcmd_type{
  RCMD_MESH_DRAW,
};

struct rcmd_t{
  enum rcmd_type type;
  union{
    mesh_t* mesh;
    mat4 model;
  }draw_mesh;
};

struct rdraw_sortable_t{
  struct rcmd_t cmd;
  uint64_t key;
};


struct rdrawlist_t{
  struct rcmd_t* data;
  size_t capacity, count;
};


#endif
