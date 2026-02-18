#include "rendering/mesh.h"



void MeshInit(mesh_t* mesh, size_t v_capacity, size_t i_capacity){
  mesh->vertex_count = 0;
  mesh->index_count = 0; 
  mesh->vertex_capacity = v_capacity;
  mesh->index_capacity = i_capacity;

  mesh->vertices = malloc(sizeof(struct vertex_t)*v_capacity);
  if (!mesh->vertices){
    fprintf(stderr, "Failed to allocate vertex array to mesh\n");
    exit(1);
  }

  mesh->indices = malloc(sizeof(GLuint)*i_capacity);
  if (!mesh->indices){
    fprintf(stderr, "Failed to allocate indices to mesh\n");
    exit(1);
  }

  glGenVertexArrays(1, &mesh->vao);
  glGenBuffers(1, &mesh->vbo);
  glGenBuffers(1, &mesh->ebo);
}

size_t MeshPushVertex(mesh_t* mesh, struct vertex_t vertex){
  if (mesh->vertex_count >= mesh->vertex_capacity){
    size_t new_capacity = mesh->vertex_capacity * 2;
    struct vertex_t* new = realloc(mesh->vertices, sizeof(struct vertex_t) * new_capacity);
    if (!new){
      fprintf(stderr, "Failed to reallocate mesh vertex array\n");
      exit(1);
    }
    mesh->vertex_capacity = new_capacity;
    mesh->vertices = new;
  }
  mesh->vertices[mesh->vertex_count] = vertex;
  return mesh->vertex_count++;
}

void MeshPushTriangle(mesh_t* mesh, GLuint i0, GLuint i1, GLuint i2){
  // Push 3 indices
  if (mesh->index_count + 3 >= mesh->index_capacity){
    size_t new_capacity = mesh->vertex_capacity * 2;
    GLuint* new = realloc(mesh->indices, sizeof(GLuint)*new_capacity);
    if (!new){
      // Write to log
      exit(1);
    }
    mesh->indices = new;
    mesh->index_capacity = new_capacity;
  }

  Vector vA = mesh->vertices[i1].pos;
  Vector vB = mesh->vertices[i2].pos;

  VectorSub(&vA, &mesh->vertices[i0].pos, &vA);
  VectorSub(&vB, &mesh->vertices[i0].pos, &vB);
  
  Vector cross;
  VectorCross(&vA, &vB, &cross);
  mesh->vertices[i0].normal = VectorZero();
  mesh->vertices[i1].normal = VectorZero();
  mesh->vertices[i2].normal = VectorZero();
  VectorAdd(&mesh->vertices[i0].normal, &cross, &mesh->vertices[i0].normal);
  VectorAdd(&mesh->vertices[i1].normal, &cross, &mesh->vertices[i1].normal);
  VectorAdd(&mesh->vertices[i2].normal, &cross, &mesh->vertices[i2].normal);


  mesh->indices[mesh->index_count++] = i0;
  mesh->indices[mesh->index_count++] = i1;
  mesh->indices[mesh->index_count++] = i2;
}


void MeshUpload(mesh_t* mesh, GLenum usage) {


    // Ensure normals are normalised
  for(int i = 0; i < mesh->vertex_count; i++){
    VectorNormalise(&mesh->vertices[i].normal);
  }


  glBindVertexArray(mesh->vao);

  glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
  glBufferData(GL_ARRAY_BUFFER, mesh->vertex_count * sizeof(struct vertex_t), mesh->vertices, usage);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->index_count * sizeof(GLuint), mesh->indices, usage);

  // Setup vertex attributes: position (3 floats), normal (3 floats), texcoord (2 floats)
  // Adjust layout according to your Vertex struct

  // Position attribute
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex_t), (void*)OFFSETOF(struct vertex_t, pos));

  // Colour attribute
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex_t), (void*)OFFSETOF(struct vertex_t, colour));
  // Texcoord attribute
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex_t), (void*)OFFSETOF(struct vertex_t, uv));
  // Normal attribute
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex_t), (void*)OFFSETOF(struct vertex_t, normal));
  glBindVertexArray(0);
}

void MeshDraw(mesh_t* mesh, GLenum mode) {
    glBindVertexArray(mesh->vao);
    glDrawElements(mode, (GLsizei)mesh->index_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void MeshDestroy(mesh_t* mesh) {
    free(mesh->vertices);
    free(mesh->indices);
    glDeleteBuffers(1, &mesh->vbo);
    glDeleteBuffers(1, &mesh->ebo);
    glDeleteVertexArrays(1, &mesh->vao);

    mesh->vertices = NULL;
    mesh->indices = NULL;
    mesh->vertex_count = 0;
    mesh->index_count = 0;
    mesh->vertex_capacity = 0;
    mesh->index_capacity = 0;
}


