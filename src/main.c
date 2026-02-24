#include <stdio.h>
#include <glad/glad.h>
#include "application.h"
#include "rendering/draw_list.h"
#include "editor/editorgui.h"
#include "inputbase.h"
#include "types/types_vector.h"
#include "rendering/camera.h"
#include "physics/rigidbody.h"
#include "physics/collider.h"

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
 
  Shader_Load(new_shader, PATH_Assets, "Shaders/defaultVert.vs", "Shaders/defaultFrag.fs");
  mesh_t triangle;
  MeshInit(&triangle, 3, 3);

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

  MeshPushTriangle(
    &triangle,
    MeshPushVertex(&triangle, v0),
    MeshPushVertex(&triangle, v1),
    MeshPushVertex(&triangle, v2)
  );

  MeshUpload(&triangle,  GL_STATIC_DRAW);
  
  camera_t main_cam;
  struct Viewport view = {0, 0, 640, 480 };
  Camera_init(&main_cam, VECTOR_ZERO, view );

  camera_t editor_cam;
  struct Viewport eView = {0, 0, 640, 480};
  Camera_init(&editor_cam, VECTOR_ONE, eView);

  rigidbody_array_t rb_arr;
  RigidbodyArray_Init(&rb_arr, 128);
  collider_array_dynamic_t collider_arr;
  ColliderArray_Init(&collider_arr, 128);
  
  EditorGui_Init(game_container.window, game_container.glContext, &editor_cam);


  while(app_running){
    poll_input(&input_state, &app_running); 
    
  
    // Rendering
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
    Shader_Use(new_shader);

    Shader_SetMat4(new_shader, "uView", gCameras[gCameraIndex]->view);
    Shader_SetMat4(new_shader, "uProj", gCameras[gCameraIndex]->projection);
    Shader_SetMat4(new_shader, "uModel", Mat4Identity());
    
    MeshDraw(&triangle, GL_TRIANGLES);
  
    EditorGui_DrawAll(game_container.window, &editor_cam, input_state.FLAG_WindowResized);


    SDL_GL_SwapWindow(game_container.window);
  }
  RigidbodyArray_Destroy(&rb_arr);
  ColliderArray_Destroy(&collider_arr);
  MeshDestroy(&triangle);
  Shader_Destroy(new_shader);
  ApplicationDestroy(&game_container);

  return 0;
}
