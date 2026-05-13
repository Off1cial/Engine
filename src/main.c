#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include "application.h"
#include "mem.h"
#include "editor/editor.h"
#include "inputbase.h"
#include "types/types_vector.h"
#include "rendering/camera.h"
#include "physics/rigidbody.h"
#include "physics/collider.h"
#include "rendering/shader.h"
#include "rendering/mesh.h"
#include "rendering/render_commands.h"
#include "rendering/draw_list.h"
#include "rendering/renderer.h"
#include "state.h"

// TOOD
//
//  REDUCE STACK USAGE
//
//
//


// Source Engine SDK 2013 https://github.com/ValveSoftware/source-sdk-2013/tree/3300848d8a25ef6403c91f82a4cd97d6daefbc06


struct mem_arena_t* gMemArena = NULL;
struct inputstate_t* gInputState = NULL;
renderer_state_t* gRendererState = NULL;
bool* gEditorActive = NULL;
editor_brush_array* gEditorBrushArray = NULL;
rigidbody_array_t* gRigidbodyArray = NULL;
collider_array_dynamic_t* gColliderArray = NULL;

int main(){
  
  const char* PATH_Assets = "../Assets/";
  
  shader_store_t shader_store = {0};
  ShaderStore_Init(&shader_store, 16);

  shader_t* shader_unlit = malloc(sizeof(shader_t));
  shader_t* shader_lit = malloc(sizeof(shader_t));
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

 
  Shader_Load(shader_unlit, PATH_Assets, "Shaders/vert_unlit.vs", "Shaders/frag_unlit.fs");
  Shader_Load(shader_lit, PATH_Assets, "Shaders/vert_lit.vs", "Shaders/frag_lit.fs");

  MeshPrimitives_Init();

  mesh_t* tri_mesh = MeshInit_FromFile("../trimesh.mesh");
  MeshRecalculateNormals(tri_mesh);
  MeshUpload(tri_mesh, GL_STATIC_DRAW);


  camera_t main_cam;
  struct Viewport view = {0, 0, 640, 480 };
  Camera_init(&main_cam, VECTOR_ZERO, view );
  gCameraIndex = 0;

  camera_t editor_cam;
  struct Viewport eView = {0, 0, 640, 480};
  Camera_init(&editor_cam, VectorInit(0, 10, 0), eView);
  editor_cam.sens = 0.04f;

  renderer_state_t renderer_state = {0};
  gRendererState = &renderer_state;
  renderer_state.active_cam = gCameras[gCameraIndex];
  renderer_state.draw_q = &gDrawQ;
  renderer_state.shader_store = &shader_store;
  renderer_state.shader_unlit = renderer_state.shader_store->shaders[ShaderStore_Add(&shader_store, shader_unlit)];
  renderer_state.shader_lit = renderer_state.shader_store->shaders[ShaderStore_Add(&shader_store, shader_lit)];
  renderer_state.light_count = 0;
  renderer_state.light_forward_count = 0;

  renderer_state.draw_normal_maps = true;
  renderer_state.wireframe = false;
  renderer_state.fullbright = false;

  // Default sun light - testing
  light_t sunLight = {
    .type = LIGHT_POINT,
    .colour = VectorInit(0.9, 0.9, 0.9),
    .direction = VectorInit(3, -1, 0),
    .intensity = 0.8f,
    .position = VectorInit(0, -8, 0),
    .radius = 50.0f,
  };

  renderer_state.lights_forward[0] = &sunLight;
  renderer_state.light_forward_count++;


  material_t* mat_marble = Material_Load("../Assets/Materials/brickmaterial.mat");
  if (!mat_marble){
    printf("mat_marble = NULL\n");
    exit(1);
  }
  Renderer_AddMaterial(&renderer_state, mat_marble);

  material_t* mat_brushhover = Material_Load("../Assets/Materials/brushhover.mat");
  Renderer_AddMaterial(&renderer_state, mat_brushhover);


  rigidbody_array_t rb_arr;
  RigidbodyArray_Init(&rb_arr, 128);
  gRigidbodyArray = &rb_arr;
  collider_array_dynamic_t collider_arr;
  ColliderArray_Init(&collider_arr, 128);
  gColliderArray = &collider_arr;
  
  editor_brush_array brush_array = {0};
  gEditorBrushArray = &brush_array;

  bool editor_active = false;
  gEditorActive = &editor_active;
  editor_state_t* editor_state = malloc(sizeof(editor_state_t));
  editor_state->camera = &editor_cam;
  EditorInit(editor_state, game_container.window, game_container.glContext);


  if (NULL == gCameras[gCameraIndex]){
    printf("Null camera\n");
  }

  state_t game_state;
  game_state.app_container = &game_container;
  game_state.arena = gMemArena;
  game_state.draw_queue = &gDrawQ;
  
  Camera_Switch(0, game_container.win_h);

  MeshDebug_WriteToFile(tri_mesh, "../meshtest.mesh");



  while(app_running){
    MEM_ARENA_RESET(gMemArena);
    RDrawQueue_Reset(&gDrawQ);
    poll_input(&input_state, &app_running, &game_container.win_w, &game_container.win_h); 
    // Toggle Editor
    if (input_state.FLAG_ToggleEditor){ EditorToggle(game_container.window); }
    
    gRendererState->lights_forward[0]->position = gRendererState->active_cam->pos;

    struct rcmd_t* rcmd = MEM_ARENA_ALLOC(gMemArena, sizeof(struct rcmd_t), alignof(struct rcmd_t));
    rcmd->type = RCMD_DRAW_MESH;
    rcmd->draw_mesh.mesh = tri_mesh;
    rcmd->draw_mesh.model = Mat4Identity();
    //rcmd->draw_mesh.view = gCameras[gCameraIndex]->view;
    //rcmd->draw_mesh.projection = gCameras[gCameraIndex]->projection;
    rcmd->draw_mesh.mode = GL_TRIANGLES;
    rcmd->draw_mesh.material = gRendererState->materials[0];
    
    // Rendering
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Test render command
    
    if (*gEditorActive){
      EditorLoop(game_container.window, &gDrawQ, &editor_cam, input_state.mx, input_state.my); 
    }
    
    /*
    for(size_t i = 0; i < brush_array.count; i++){
      EditorBrush_Draw(&brush_array.brushes[i], &gDrawQ, &editor_cam);
    }
    */

    RDrawQueue_Push(&gDrawQ, rcmd);
   
  
     

    RDrawQueue_Execute(&gDrawQ);
    SDL_GL_SwapWindow(game_container.window);
  }
  RigidbodyArray_Destroy(&rb_arr);
  ColliderArray_Destroy(&collider_arr);
  MeshPrimitives_Destroy();
  MeshDestroy(tri_mesh);
  //free(tri_mesh);
  //EditorDestroy(editor_state);
  Renderer_Destroy(gRendererState);
  MEM_ARENA_DESTROY(gMemArena);
  ApplicationDestroy(&game_container);

  return 0;
}
