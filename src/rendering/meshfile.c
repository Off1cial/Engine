#include "rendering/meshfile.h"


bool MeshFile_WriteMany(const char* filepath, mesh_t** meshes, size_t mesh_count){
  FILE* fptr = fopen(filepath, "wb");

  if (!fptr){
    fprintf(stderr, "[MESH]: Failed to open file for writing: %s\n", filepath);
    return false;
  }

  struct mesh_file_header file_header = {
    .magic = "MSH0",
    .mesh_count = mesh_count
  };
  
  if (fwrite(&file_header, sizeof(file_header), 1, fptr) != 1){
    fprintf(stderr, "[MESH]: Failed to write file header to %s\n", filepath);
    fclose(fptr);
    return false;
  } 
  
  for (size_t i = 0; i < mesh_count; i++){
    mesh_t* mesh = meshes[i];

    struct mesh_chunk_header chunk = 
      {
        (uint32_t)mesh->vertex_count,
        (uint32_t)mesh->index_count
      };

    if (fwrite(&chunk, sizeof(chunk), 1, fptr) != 1){
      fprintf(stderr, "[MESH]: Failed to mesh chunk header to %s\n", filepath);
      fclose(fptr);
      return false;
    }

    if (mesh->vertex_count > 0){
      if (fwrite(mesh->vertices, sizeof(struct vertex_t), mesh->vertex_count, fptr) != mesh->vertex_count){
        fprintf(stderr, "[MESH]: Failed to write mesh vertices to %s\n", filepath);
        fclose(fptr);
        return false;
      }
    }

    if (mesh->index_count > 0){
      if (fwrite(mesh->indices, sizeof(GLuint), mesh->index_count, fptr) != mesh->index_count){
        fprintf(stderr, "[MESH]: Failed to write mesh indices to %s\n", filepath);
        fclose(fptr);
        return false;
      }
    }
  }


  fclose(fptr);
  return true;
}


mesh_t* MeshFile_ReadMany(const char* filepath, size_t* mesh_count_out){
  FILE* fptr = fopen(filepath, "rb");

  if (!fptr){
    fprintf(stderr, "[MESH]: Failed to open file for reading: %s\n", filepath);
    return NULL;
  }

  struct mesh_file_header file_header;

  if (fread(&file_header, sizeof(file_header), 1, fptr) != 1){
    fprintf(stderr, "[MESH]: Failed to read file header from %s\n", filepath);
    fclose(fptr);
    return NULL;
  }

  if (memcmp(file_header.magic, "MSH0", 4) != 0){
    fprintf(stderr, "[MESH]: Invalid file magic: %s\n", filepath);
    fclose(fptr);
    return NULL;
  }

  mesh_t* meshes = calloc(file_header.mesh_count, sizeof(file_header.mesh_count));


}
