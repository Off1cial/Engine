#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include "application.h"
#include "mem.h"
#include "editor/editor.h"
#include "editor/brush_render.h"
#include "inputbase.h"
#include "types/types_vector.h"
#include "rendering/camera.h"
#include "physics/rigidbody.h"
#include "physics/collider.h"
#include "rendering/shader.h"
#include "rendering/mesh.h"
#include "rendering/render_commands.h"
#include "rendering/draw_list.h"
#include "state.h"

// TOOD
//
//  REDUCE STACK USAGE
//
//
//



struct mem_arena_t* gMemArena = NULL;
struct inputstate_t* gInputState = NULL;
brush_array_t* gEditorBrushArray = NULL;
bool* gEditorActive = NULL;

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
  gInputState = &input_state;
  app_running = !ApplicationInit(&game_container, 640, 480);
  SDL_GL_SetSwapInterval(1);

  struct mem_arena_t mem_arena;
  gMemArena = &mem_arena;
  MEM_ARENA_INIT(gMemArena);
  MEM_ARENA_RESET(gMemArena);

  rdrawqueue_t gDrawQ;
  RDrawQueue_Init(&gDrawQ, 128);

 
  Shader_Load(new_shader, PATH_Assets, "Shaders/defaultVert.vs", "Shaders/defaultFrag.fs");
  SHADER_default_shader = new_shader; 

  MeshPrimitives_Init();

  mesh_t* tri_mesh = malloc(sizeof(mesh_t));
  MeshInit( tri_mesh, 3, 3);

  struct vertex_t v0 = {
    .pos = {-0.5f, 0.0f, 5.0f},
    .colour = {1.0f, 0.0f, 0.0f},
  };
  struct vertex_t v1 = {
    .pos = {-0.0f, 0.5f, 5.0f},
    .colour = {0.0f, 1.0f, 0.0f},
  };
  struct vertex_t v2 = {
    .pos = {0.5f, 0.0f, 5.0f},
    .colour = {0.0f, 0.0f, 1.0f},
  };


  GLuint i0 = MeshPushVertex(tri_mesh, v0);
  GLuint i1 = MeshPushVertex(tri_mesh, v1);
  GLuint i2 = MeshPushVertex(tri_mesh, v2);

  MeshPushTriangle(tri_mesh, i0, i1, i2);
  MeshUpload(tri_mesh, GL_STATIC_DRAW);

  
  camera_t main_cam;
  struct Viewport view = {0, 0, 640, 480 };
  Camera_init(&main_cam, VectorInit(0, 0, 0), view );
  gCameraIndex = 0;

  camera_t editor_cam;
  struct Viewport eView = {0, 0, 640, 480};
  Camera_init(&editor_cam, VECTOR_ONE, eView);

  rigidbody_array_t rb_arr;
  RigidbodyArray_Init(&rb_arr, 128);
  collider_array_dynamic_t collider_arr;
  ColliderArray_Init(&collider_arr, 128);
  

  bool editor_active = false;
  gEditorActive = &editor_active;
  brush_array_t* editor_brush_array = malloc(sizeof(brush_array_t));
  gEditorBrushArray = editor_brush_array;
  editor_state_t* editor_state = malloc(sizeof(editor_state_t));
  editor_state->camera = &editor_cam;
  EditorInit(editor_state, game_container.window, game_container.glContext);

  if (NULL == gCameras[gCameraIndex]){
    printf("Null camera\n");
  }
  
  EditorBrush_Create(editor_brush_array,  VectorInit(0,0, 3), VECTOR_ONE);
  EditorBrush_Create(editor_brush_array,  VectorInit(2,0, 3), VectorInit(0.5f, 0.5f, 0.5f));

  state_t game_state;
  game_state.app_container = &game_container;
  game_state.arena = gMemArena;
  game_state.draw_queue = &gDrawQ;
  
  //Camera_Switch(0, game_container.win_h);

  while(app_running){
    MEM_ARENA_RESET(gMemArena);
    RDrawQueue_Reset(&gDrawQ);
    poll_input(&input_state, &app_running, &game_container.win_w, &game_container.win_h); 
    // Toggle Editor
    if (input_state.FLAG_ToggleEditor){ EditorToggle(game_container.window); }
  

    struct rcmd_t* rcmd = MEM_ARENA_ALLOC(gMemArena, sizeof(struct rcmd_t), alignof(struct rcmd_t));
    rcmd->type = RCMD_DRAW_MESH;
    rcmd->draw_mesh.mesh = tri_mesh;
    rcmd->draw_mesh.model = Mat4Identity();
    rcmd->draw_mesh.view = gCameras[gCameraIndex]->view;
    rcmd->draw_mesh.projection = gCameras[gCameraIndex]->projection;
    rcmd->draw_mesh.mode = GL_TRIANGLES;
    rcmd->draw_mesh.material_index = 0;
    rcmd->draw_mesh.shader = new_shader;
    
  
    // Rendering
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Test render command
    if (*gEditorActive) {EditorLoop(game_container.window, &gDrawQ, &editor_cam); }
    RDrawQueue_Push(&gDrawQ, rcmd);
    
     

    RDrawQueue_Execute(&gDrawQ);
    SDL_GL_SwapWindow(game_container.window);
  }
  RigidbodyArray_Destroy(&rb_arr);
  ColliderArray_Destroy(&collider_arr);
  MeshPrimitives_Destroy();
  MeshDestroy(tri_mesh);
  free(tri_mesh);
  EditorDestroy(editor_state);
  Shader_Destroy(new_shader);
  RDrawQueue_Destroy(&gDrawQ);
  MEM_ARENA_DESTROY(gMemArena);
  ApplicationDestroy(&game_container);

  return 0;
}
