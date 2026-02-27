#include "rendering/mesh.h"
#include <string.h>

mesh_t* MESH_PRIMITIVES[MESH_PRIMITIVES_COUNT];

void MeshDebug_PrintVertex(struct vertex_t *v)
{
  printf("{%0.3f, %0.3f, %0.3f}\n", v->pos.x, v->pos.y, v->pos.z);
}

void MeshDebug_PrintVertices(mesh_t *mesh)
{
  if (!mesh)
  {
    printf("[MeshDebug]: Mesh is null\n");
    return;
  }
  for (size_t v = 0; v < mesh->vertex_count; v++)
  {
    struct vertex_t vert = mesh->vertices[v];
    printf("V%zu: {%0.3f, %0.3f, %0.3f}\n", v, vert.pos.x, vert.pos.y, vert.pos.z);
  }
}

void MeshInit(mesh_t *mesh, size_t v_capacity, size_t i_capacity)
{
  memset(mesh, 0, sizeof(mesh_t));

  mesh->vertex_count = 0;
  mesh->index_count = 0;
  mesh->vertex_capacity = v_capacity;
  mesh->index_capacity = i_capacity;

  mesh->vertices = malloc(sizeof(struct vertex_t) * v_capacity);
  if (!mesh->vertices)
  {
    fprintf(stderr, "Failed to allocate vertex array to mesh\n");
    exit(1);
  }
  memset(mesh->vertices, 0, sizeof(struct vertex_t) * v_capacity);
  mesh->indices = malloc(sizeof(GLuint) * i_capacity);
  if (!mesh->indices)
  {
    fprintf(stderr, "Failed to allocate indices to mesh\n");
    exit(1);
  }

  glGenVertexArrays(1, &mesh->vao);
  glGenBuffers(1, &mesh->vbo);
  glGenBuffers(1, &mesh->ebo);
}

size_t MeshPushVertex(mesh_t *mesh, struct vertex_t vertex)
{
  if (mesh->vertex_count >= mesh->vertex_capacity)
  {
    size_t new_capacity = mesh->vertex_capacity * 2;
    struct vertex_t *new = realloc(mesh->vertices, sizeof(struct vertex_t) * new_capacity);
    if (!new)
    {
      fprintf(stderr, "Failed to reallocate mesh vertex array\n");
      exit(1);
    }
    mesh->vertex_capacity = new_capacity;
    mesh->vertices = new;
  }
  mesh->vertices[mesh->vertex_count] = vertex;
  return mesh->vertex_count++;
}

void MeshPushTriangle(mesh_t *mesh, GLuint i0, GLuint i1, GLuint i2)
{
  // Push 3 indices
  if (mesh->index_count + 3 > mesh->index_capacity)
  {
    size_t new_capacity = mesh->index_capacity * 2;
    GLuint *new = realloc(mesh->indices, sizeof(GLuint) * new_capacity);
    if (!new)
    {
      // Write to log
      exit(1);
    }
    mesh->indices = new;
    mesh->index_capacity = new_capacity;
  }

  Vector vA = mesh->vertices[i1].pos;
  Vector vB = mesh->vertices[i2].pos;

  vA = VectorSub(vA, mesh->vertices[i0].pos);
  vB = VectorSub(vB, mesh->vertices[i0].pos);

  Vector cross = VectorCross(vA, vB);
  mesh->vertices[i0].normal = VectorAdd(mesh->vertices[i0].normal, cross);
  mesh->vertices[i1].normal = VectorAdd(mesh->vertices[i1].normal, cross);
  mesh->vertices[i2].normal = VectorAdd(mesh->vertices[i2].normal, cross);

  mesh->indices[mesh->index_count++] = i0;
  mesh->indices[mesh->index_count++] = i1;
  mesh->indices[mesh->index_count++] = i2;
}

void MeshUpload(mesh_t *mesh, GLenum usage)
{

  // Ensure normals are normalised
  for (int i = 0; i < mesh->vertex_count; i++)
  {
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
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex_t), (void *)OFFSETOF(struct vertex_t, pos));

  // Colour attribute
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex_t), (void *)OFFSETOF(struct vertex_t, colour));
  // Texcoord attribute
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex_t), (void *)OFFSETOF(struct vertex_t, uv));
  // Normal attribute
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex_t), (void *)OFFSETOF(struct vertex_t, normal));
  glBindVertexArray(0);
}

void MeshDraw(mesh_t *mesh, GLenum mode)
{
  glBindVertexArray(mesh->vao);
  glDrawElements(mode, (GLsizei)mesh->index_count, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void MeshDestroy(mesh_t *mesh)
{
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

// DEBUG LINES

struct dbg_container_t dbg_cont;

void DebugLines_Init()
{
  dbg_cont.capacity = 128;
  dbg_cont.count = 0;
  dbg_cont.vertices = malloc(sizeof(struct vertex_debug_t) * dbg_cont.capacity);
}

void DebugLines_Push(Vector a, Vector b, Vector4 colour)
{
  if (dbg_cont.count >= dbg_cont.capacity)
  {
    dbg_cont.capacity *= 2;
  }

  dbg_cont.vertices[dbg_cont.count++] = (struct vertex_debug_t){a, colour};
  dbg_cont.vertices[dbg_cont.count++] = (struct vertex_debug_t){b, colour};
}

void MeshPrimitives_Init()
{
  // Allocate the mesh
  mesh_t *mCube = malloc(sizeof(mesh_t));
  MeshInit(mCube, 8, 12 * 3); // 8 vertices, 12 triangles

  // Define 8 cube corners (centered at origin, size 1)
  Vector cube_vertices[8] = {
      {-0.5f, -0.5f, -0.5f}, // 0: left-bottom-back
      {0.5f, -0.5f, -0.5f},  // 1: right-bottom-back
      {0.5f, 0.5f, -0.5f},   // 2: right-top-back
      {-0.5f, 0.5f, -0.5f},  // 3: left-top-back
      {-0.5f, -0.5f, 0.5f},  // 4: left-bottom-front
      {0.5f, -0.5f, 0.5f},   // 5: right-bottom-front
      {0.5f, 0.5f, 0.5f},    // 6: right-top-front
      {-0.5f, 0.5f, 0.5f}    // 7: left-top-front
  };

  GLuint inds[8];
  for (int i = 0; i < 8; i++)
  {
    struct vertex_t v = {
        .pos = cube_vertices[i],
        .colour = VectorInit(1.0f, 0.4f, 0.0f)};
    inds[i] = MeshPushVertex(mCube, v);
  }
  // Back face (Z-)
  MeshPushTriangle(mCube, inds[0], inds[1], inds[2]);
  MeshPushTriangle(mCube, inds[2], inds[3], inds[0]);

  // Front face (Z+)
  MeshPushTriangle(mCube, inds[4], inds[5], inds[6]);
  MeshPushTriangle(mCube, inds[6], inds[7], inds[4]);

  // Left face (X-)
  MeshPushTriangle(mCube, inds[0], inds[3], inds[7]);
  MeshPushTriangle(mCube, inds[7], inds[4], inds[0]);

  // Right face (X+)
  MeshPushTriangle(mCube, inds[1], inds[5], inds[6]);
  MeshPushTriangle(mCube, inds[6], inds[2], inds[1]);

  // Top face (Y+)
  MeshPushTriangle(mCube, inds[3], inds[2], inds[6]);
  MeshPushTriangle(mCube, inds[6], inds[7], inds[3]);

  // Bottom face (Y-)
  MeshPushTriangle(mCube, inds[0], inds[4], inds[5]);
  MeshPushTriangle(mCube, inds[5], inds[1], inds[0]);

  /*
  // Define 12 triangles (two per cube face)
  int faces[12][3] = {
      {0, 1, 2}, {0, 2, 3}, // back
      {4, 5, 6},
      {4, 6, 7}, // front
      {0, 4, 7},
      {0, 7, 3}, // left
      {1, 5, 6},
      {1, 6, 2}, // right
      {3, 2, 6},
      {3, 6, 7}, // top
      {0, 1, 5},
      {0, 5, 4} // bottom
  };

  for (int t = 0; t < 12; t++)
  {
    MeshPushTriangle(mCube, inds[faces[t][0]], inds[faces[t][1]], inds[faces[t][2]]);
  }
  */

  // Upload to GPU once (GL_STATIC_DRAW)
  MeshUpload(mCube, GL_STATIC_DRAW);
  MESH_PRIMITIVES[MESH_PRIMITIVE_CUBE] = mCube;
  // Store globally so brushes can reference it
}


void MeshPrimitives_Destroy(){
  for (int i = 0; i < MESH_PRIMITIVES_COUNT; i++){
    MeshDestroy(MESH_PRIMITIVES[i]);
    free(MESH_PRIMITIVES[i]);
  }
}
