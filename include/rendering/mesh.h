#pragma once
#include "headers.h"
#include "types/types_vector.h"
#include <glad/glad.h>


#define MESH_PUSH_VERTEX_FAIL ((size_t)-1)
#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))

#pragma pack(push, 1)
struct vertex_t{
  Vector pos;
  Vector colour;
  Vector normal;
  Vector tangent;
  Vector2 uv;
};
#pragma pack(pop)

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

  bool nan;
} mesh_t;

void MeshInit(mesh_t* mesh, size_t v_capacity, size_t i_capacity);
mesh_t* MeshInit_FromFile(const char* filepath);
size_t MeshPushVertex(mesh_t* mesh, struct vertex_t vertex);
void MeshPushTriangle(mesh_t* mesh, GLuint i0, GLuint i1, GLuint i2);
void MeshRecalculateNormals(mesh_t* mesh);
void MeshUpload(mesh_t* mesh, GLenum usage);
void MeshDraw(mesh_t* mesh, GLenum mode);
void MeshDestroy(mesh_t* mesh);
void MeshReset(mesh_t* mesh);



struct vertex_t VectorToVertex(Vector a, float col[3]);
struct vertex_t MakeVertex(Vector pos, Vector colour, Vector2 uv);

void MeshDebug_PrintVertex(struct vertex_t* v);
void MeshDebug_PrintVertices(mesh_t* mesh);
void MeshDebug_WriteToFile(mesh_t* mesh, const char* filepath);


// Turn the above into an enum?

typedef enum meshprimitive_t{
  MESH_PRIMITIVE_CUBE = 0,
  MESH_PRIMITIVES_COUNT
} meshprimitive_t;

extern mesh_t* MESH_PRIMITIVES[MESH_PRIMITIVES_COUNT];


void MeshPrimitives_Init();
void MeshPrimitives_Destroy();


mesh_t* MeshFromPlane(plane_t plane);




