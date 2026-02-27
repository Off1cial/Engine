#include "editor/brush_render.h"

/*

void get_winding_vertices(winding_t* in, struct vertex_t* out, size_t vertex_count){
  if (vertex_count != in->count){
    fprintf(stderr, "[BrushRender]: Winding vertex array differs in size to vertex_t destination\n");
    exit(1);
  }
  for( int v = 0; v < vertex_count; v++ ){
    struct vertex_t vertex = {
      .pos = in->points[v],
      .colour = VECTOR_ONE
    };
    out[v] = vertex;
  }
}


mesh_t* EditorBrush_CreateRenderMesh(brush_array_t* arr, size_t brush, struct mem_arena_t* arena){
  if (!arr || !arena){ return NULL; }
  
  mesh_t* mesh = MEM_ARENA_ALLOC(arena, sizeof(mesh_t), alignof(mesh_t));

  for (size_t s = 0; s < arr->side_count[brush]; s++){

    size_t side = s + arr->side_start[brush];
    winding_t* win = arr->sides[side].winding;
    struct vertex_t* side_vertices = MEM_ARENA_ALLOC(arena, sizeof(struct vertex_t) * win->count, alignof(struct vertex_t));
    // Convert winding points to vertex_t
    get_winding_vertices(win, side_vertices, win->count);
    // Push vertices and form triangles for the side
    GLuint* inds = MEM_ARENA_ALLOC(arena, sizeof(GLuint) * win->count, alignof(GLuint));
    for (size_t v = 0; v < win->count; v++){
      // Push vertices
      inds[v] = MeshPushVertex(mesh, side_vertices[v]);
    }
    // Form triangle
    for (size_t v = 1; v < win->count - 1; v++){
      MeshPushTriangle(mesh, inds[0], inds[v], inds[v+1]);
    }
  }
  return mesh;
}


void EditorBrush_DrawAll(brush_array_t* arr, state_t* state, camera_t* camera ){
  for (size_t b = 0; b < arr->brush_count; b++){
    if (arr->editor_meshes[b] == NULL){ continue; }
  
    // Allocate on arena
    struct rcmd_t* cmd = MEM_ARENA_ALLOC(
      state->arena, sizeof(struct rcmd_t),
      alignof(struct rcmd_t)
    );
    // Prepare command
    // Assumes default shader, no material at this point in time
    cmd->type = RCMD_DRAW_MESH;
    cmd->draw_mesh.mesh = arr->editor_meshes[b];
    cmd->draw_mesh.shader = SHADER_default_shader;
    cmd->draw_mesh.view = camera->view;
    cmd->draw_mesh.projection = camera->projection;
    cmd->draw_mesh.model = Mat4Identity();
    cmd->draw_mesh.material_index = 0;
    cmd->draw_mesh.mode = GL_TRIANGLES;

    RDrawQueue_Push(state->draw_queue, cmd);
  }
}
*/
