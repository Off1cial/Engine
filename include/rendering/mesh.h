#pragma once
#include "headers.h"
#include "types/types_vector.h"
#include <glad/glad.h>


#define MESH_PUSH_VERTEX_FAIL ((size_t)-1)
#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))

struct vertex_t{
  Vector pos;
  Vector colour;
  Vector normal;
  Vector2 uv;
};

// Exclusively for deubg lines
struct vertex_debug_t{ 
  Vector pos;
  Vector4 colour;
};

struct dbg_container_t{
  struct vertex_debug_t* vertices;
  size_t count, capacity;
};

typedef struct {
  GLuint vao, vbo, ebo;
  size_t index_count;
  size_t index_capacity;
  size_t vertex_count;
  size_t vertex_capacity;

  struct vertex_t* vertices;
  GLuint* indices;
} mesh_t;

void MeshInit(mesh_t* mesh, size_t v_capacity, size_t i_capacity);
mesh_t* MeshInit_FromFile(const char* filepath);
size_t MeshPushVertex(mesh_t* mesh, struct vertex_t vertex);
void MeshPushTriangle(mesh_t* mesh, GLuint i0, GLuint i1, GLuint i2);
void MeshUpload(mesh_t* mesh, GLenum usage);
void MeshDraw(mesh_t* mesh, GLenum mode);
void MeshDestroy(mesh_t* mesh);
void MeshReset(mesh_t* mesh);

void DebugLines_Init();
void DebugLines_Push(Vector a, Vector b, Vector4 colour);
void DebugLines_Upload();
void DebugLines_Draw();


struct vertex_t VectorToVertex(Vector a, float col[3]);

void MeshDebug_PrintVertex(struct vertex_t* v);
void MeshDebug_PrintVertices(mesh_t* mesh);
void MeshDebug_WriteToFile(mesh_t* mesh, const char* filepath);

#define MESH_PRIMITIVES_COUNT 1
#define MESH_PRIMITIVE_CUBE 0


extern mesh_t* MESH_PRIMITIVES[MESH_PRIMITIVES_COUNT];


void MeshPrimitives_Init();
void MeshPrimitives_Destroy();




