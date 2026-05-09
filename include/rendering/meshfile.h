#ifndef MESHFILE_H
#define MESHFILE_H

#include "rendering/mesh.h"


struct mesh_file_header{
  char magic[4];
  uint32_t mesh_count;
};

struct mesh_chunk_header{
  //char name[64];
  uint32_t vertex_count;
  uint32_t index_count;
};

bool MeshFile_WriteMany(const char* filepath, mesh_t** meshes, size_t mesh_count);

mesh_t* MeshFile_ReadMany(const char* filepath, size_t* mesh_count_out);

#endif
