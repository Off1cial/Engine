#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include "application.h"
#include "mem.h"
#include "editor/editorgui.h"
#include "inputbase.h"
#include "types/types_vector.h"
#include "rendering/camera.h"
#include "physics/rigidbody.h"
#include "physics/collider.h"
#include "rendering/shader.h"
#include "rendering/mesh.h"
#include "rendering/render_commands.h"
#include "rendering/draw_list.h"

int main(){
  
  const char* PATH_Assets = "../Assets/";
  

  bool SHADER_isDefaultLoaded = false;
  bool SHADER_isDefaultLitLoaded = false;
  bool SHADER_isDefaultUILoaded = false;
  bool SHADER_isDefaultBillboardLoaded = false;
  

  shader_t* new_shader = malloc(sizeof(shader_t));
  int app_running;
  struct container_t game_container;
  struct inputstate_t input_state;
  app_running = !ApplicationInit(&game_container, 640, 480);
  SDL_GL_SetSwapInterval(1);

  struct mem_arena_t gMemArena;
  MEM_ARENA_INIT(&gMemArena);
  
  rdrawqueue_t gDrawQ;
  RDrawQueue_Init(&gDrawQ, 128);

 
  Shader_Load(new_shader, PATH_Assets, "Shaders/defaultVert.vs", "Shaders/defaultFrag.fs");
  SHADER_default_shader = new_shader; 

  mesh_t* tri_mesh = malloc(sizeof(mesh_t));
  MeshInit( tri_mesh, 3, 3);

  struct vertex_t v0 = {
    .pos = {-0.5f, 0.0f, 3.0f},
    .colour = {1.0f, 0.0f, 0.0f},
  };
  struct vertex_t v1 = {
    .pos = {-0.0f, 0.5f, 3.0f},
    .colour = {0.0f, 1.0f, 0.0f},
  };
  struct vertex_t v2 = {
    .pos = {0.5f, 0.0f, 3.0f},
    .colour = {0.0f, 0.0f, 1.0f},
  };


  GLuint i0 = MeshPushVertex(tri_mesh, v0);
  GLuint i1 = MeshPushVertex(tri_mesh, v1);
  GLuint i2 = MeshPushVertex(tri_mesh, v2);

  MeshPushTriangle(tri_mesh, i0, i1, i2);
  MeshUpload(tri_mesh, GL_STATIC_DRAW);

  printf("Vertice count: %zu\n", tri_mesh->vertex_count);
  
  camera_t main_cam;
  struct Viewport view = {0, 0, 640, 480 };
  Camera_init(&main_cam, VECTOR_ZERO, view );
  gCameras[gCameraCount++] = &main_cam;
  gCameraIndex = 0;

  camera_t editor_cam;
  struct Viewport eView = {0, 0, 640, 480};
  Camera_init(&editor_cam, VECTOR_ONE, eView);

  rigidbody_array_t rb_arr;
  RigidbodyArray_Init(&rb_arr, 128);
  collider_array_dynamic_t collider_arr;
  ColliderArray_Init(&collider_arr, 128);
  
  EditorGui_Init(game_container.window, game_container.glContext, &editor_cam);

  if (NULL == gCameras[gCameraIndex]){
    printf("Null camera\n");
  }
  

  while(app_running){
    MEM_ARENA_RESET(&gMemArena);
    RDrawQueue_Reset(&gDrawQ);
    poll_input(&input_state, &app_running); 
    
    struct rcmd_t rcmd;
    rcmd.type = RCMD_DRAW_MESH;
    rcmd.draw_mesh.mesh = tri_mesh;
    rcmd.draw_mesh.model = Mat4Identity();
    rcmd.draw_mesh.view = gCameras[gCameraIndex]->view;
    rcmd.draw_mesh.projection = gCameras[gCameraIndex]->projection;
    rcmd.draw_mesh.mode = GL_TRIANGLES;
    rcmd.draw_mesh.material_index = 0;
    rcmd.draw_mesh.shader = new_shader;
    
  
    // Rendering
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Test render command

    RDrawQueue_Push(&gDrawQ, &rcmd);
    
    /* 
    Shader_Use(new_shader);
    Shader_SetMat4(new_shader, "uView", main_cam.view);
    Shader_SetMat4(new_shader, "uProj", main_cam.projection);
    Shader_SetMat4(new_shader, "uModel", Mat4Identity());
     
    //EditorGui_DrawAll(game_container.window, &editor_cam, input_state.FLAG_WindowResized);
    MeshDraw(tri_mesh, GL_TRIANGLES); 
    */

    RDrawQueue_Execute(&gDrawQ);
    SDL_GL_SwapWindow(game_container.window);
  }
  RigidbodyArray_Destroy(&rb_arr);
  ColliderArray_Destroy(&collider_arr);
  MeshDestroy(tri_mesh);
  free(tri_mesh);
  Shader_Destroy(new_shader);
  RDrawQueue_Destroy(&gDrawQ);
  MEM_ARENA_DESTROY(&gMemArena);
  ApplicationDestroy(&game_container);

  return 0;
}
