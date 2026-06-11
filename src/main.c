#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include "application.h"
#include "mem.h"
#include "editor/editor.h"
#include "editor/editorcmd.h"
#include "editor/bsp.h"
#include "inputbase.h"
#include "types/types_vector.h"
#include "rendering/camera.h"
#include "physics/rigidbody.h"
#include "physics/collider.h"
#include "rendering/text.h"
#include "rendering/shader.h"
#include "rendering/mesh.h"
#include "rendering/render_commands.h"
#include "rendering/draw_list.h"
#include "rendering/renderer.h"
#include "console/console.h"
#include "console/consolegui.h"
#include "state.h"

#include "player/player.h"
#include "player/pmove.h"

// TOOD
//
//  REDUCE STACK USAGE
//
//
//

// CURRENT STATE - 08/06/2026 - THE BSP HAS TROUBLE DEFINING SOLID SPACE BETWEEN TWO BRUSH ROOMS, AS IN ONE INSIDE THE OTHER - fixed. make the skybox an entity, retard.

// Source Engine SDK 2013 https://github.com/ValveSoftware/source-sdk-2013/tree/3300848d8a25ef6403c91f82a4cd97d6daefbc06

struct mem_arena_t *gMemArena = NULL;
struct inputstate_t *gInputState = NULL;
renderer_state_t *gRendererState = NULL;
bool *gEditorActive = NULL;
editor_brush_array *gEditorBrushArray = NULL;
rigidbody_array_t *gRigidbodyArray = NULL;
collider_array_dynamic_t *gColliderArray = NULL;
bsp_tree_t *BSP_ACTIVE_TREE = NULL;

bool gCursorLocked = false;

int main()
{

  const char *PATH_Assets = "../Assets/";

  shader_store_t shader_store = {0};
  ShaderStore_Init(&shader_store, 16);

  shader_t *shader_unlit = malloc(sizeof(shader_t));
  shader_t *shader_lit = malloc(sizeof(shader_t));
  int app_running;
  struct container_t game_container;
  APPLIACTION_CONTAINER = &game_container;
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

  // mesh_t* tri_mesh = MeshInit_FromFile("../trimesh.mesh");
  // MeshRecalculateNormals(tri_mesh);
  // MeshUpload(tri_mesh, GL_STATIC_DRAW);

  camera_t main_cam;
  struct Viewport view = {.x = 0, .y = 0, .w = APPLIACTION_CONTAINER->win_w, .h = APPLIACTION_CONTAINER->win_h};
  Camera_init(&main_cam, VectorInit(0, 0, 0), view);
  gCameraIndex = 0;
  main_cam.sens = 0.1f;

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
  renderer_state.textbatches2d_count = 0;
  renderer_state.textbatches3d_count = 0;

  renderer_state.draw_normal_maps = true;
  renderer_state.wireframe = false;
  renderer_state.fullbright = false;

  // Default sun light - testing
  light_t pointLight = {
    .type = LIGHT_POINT,
    .colour = VectorInit(0.55, 0.55, 0.5),
    .direction = VectorInit(3, -1, 0),
    .intensity = 0.8f,
    .position = VectorInit(0, 560, 0),
    .radius = 60.0f,
  };


  // Default spot light
  light_t spotLight = {
      .type = LIGHT_SPOT,
      .colour = VectorInit(0.7, 0.7, 0.8),
      .direction = main_cam.front,
      .intensity = 1.0f,
      .position = VectorInit(0, -8, 0),
      .radius = 100.0f,
      .cutoff = cosf(RAD(15)) //float cutoff = cosf(RAD(degrees * 0.5f));
  };

  renderer_state.lights_forward[0] = &pointLight;
  renderer_state.light_forward_count++;
  renderer_state.lights_forward[1] = &spotLight;
  renderer_state.light_forward_count++;

  

  material_t *mat_marble = Material_Load("../Assets/Materials/brickmaterial.mat");
  if (!mat_marble)
  {
    printf("mat_marble = NULL\n");
    exit(1);
  }
  Renderer_AddMaterial(&renderer_state, mat_marble);

  material_t *mat_brushhover = Material_Load("../Assets/Materials/brushhover.mat");
  Renderer_AddMaterial(&renderer_state, mat_brushhover);
  material_t *mat_debugplane = Material_Load("../Assets/Materials/debugplane.mat");
  Renderer_AddMaterial(&renderer_state, mat_debugplane);
  material_t *mat_skyboxplaceholder = Material_Load("../Assets/Materials/Editor/skybox.mat");
  Renderer_AddMaterial(&renderer_state, mat_skyboxplaceholder);

  TTF_Init();
  font_t *FONT_TF2BUILD = Font_Load("../Assets/Fonts/tf2build.ttf", 28);

  /*
  rigidbody_array_t rb_arr;
  RigidbodyArray_Init(&rb_arr, 128);
  gRigidbodyArray = &rb_arr;
  collider_array_dynamic_t collider_arr;
  ColliderArray_Init(&collider_arr, 128);
  gColliderArray = &collider_arr;
  */
  Console_Init();
  editor_brush_array brush_array = {0};
  gEditorBrushArray = &brush_array;

  bool editor_active = false;
  gEditorActive = &editor_active;
  editor_state_t *editor_state = malloc(sizeof(editor_state_t));
  editor_state->camera = &editor_cam;
  EditorInit(editor_state, game_container.window, game_container.glContext);

  if (NULL == gCameras[gCameraIndex])
  {
    printf("Null camera\n");
  }

  state_t game_state;
  game_state.app_container = &game_container;
  game_state.arena = gMemArena;
  game_state.draw_queue = &gDrawQ;

  Camera_Switch(0, game_container.win_w, game_container.win_h);

  // MeshDebug_WriteToFile(tri_mesh, "../meshtest.mesh");

  // Test rooms
  EditorCreate_BrushRoom(gEditorBrushArray, VectorInit(-150, 500, -150), VectorInit(150, 800, 150), 0, 0);
  EditorCreate_BrushRoom(gEditorBrushArray, VectorInit(-200, 0, -150), VectorInit(100, 300, 150), 0, 0);
  EditorCreate_BrushRoom(gEditorBrushArray, VectorInit(-800, -900, -800), VectorInit(800, 900, 800), 3, 1); // skybox

  Console_WriteLine("Console is working?", CONSOLE_LINE_WARNING);

  PlayerArrayInit(1);
  PlayerCreate(VectorInit(0, 540, 0), &main_cam);
  BSP_ACTIVE_TREE = BSP_Compile();
  while (app_running)
  {

    MEM_ARENA_RESET(gMemArena);
    RDrawQueue_Reset(&gDrawQ);
    poll_input(&input_state, &app_running, &game_container.win_w, &game_container.win_h, game_container.window);
    if (input_state.kCurrent[SDL_SCANCODE_K] && !input_state.kPrevious[SDL_SCANCODE_K])
    {
      BSP_ACTIVE_TREE = BSP_Compile();
    }
    // Toggle Editor
    if (input_state.FLAG_ToggleEditor)
    {
      EditorToggle(game_container.window);
    }

    //gRendererState->lights_forward[0]->position = main_cam.pos;
    gRendererState->lights_forward[1]->position = main_cam.pos;
    gRendererState->lights_forward[1]->direction = main_cam.front;
    //gRendererState->lights_forward[0]->direction = main_cam.front;
    // Rendering
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Text_AddString(FONT_TF2BUILD, "TF2", VectorInit(12, 12, 0), (Vector4){0.5, 0.1, 0.1, 1}, true);
    Text_AddString(FONT_TF2BUILD, "TF2", VectorInit(10, 10, 0), (Vector4){1, 1, 1, 1}, true);
    
    ImGui_StartFrame();
    // Test render command
    if (!(*gEditorActive))
    {
      if (input_state.FLAG_WindowResized)
      {
        Camera_UpdateViewport(gPlayers->camera[0], 0, 0, game_container.win_w, game_container.win_h, game_container.win_h, 1);
      }
      gRendererState->active_cam = gPlayers->camera[0];
      //gPlayers->camera[0]->pos = (Vector){gPlayers->x[0] + 20, gPlayers->y[0] + 20, gPlayers->z[0]};
      PlayerLook(0, gInputState->mx_rel, gInputState->my_rel);
      PlayerMove(0, 320, 160, 0, 20, 32, 0.01);
      // printf("Player Y: %.2f, grounded: %d\n", gPlayers->y[0], gPlayers->grounded[0]);
      if (BSP_ACTIVE_TREE)
      {
        R_DrawBSPFaces(&main_cam, BSP_ACTIVE_TREE->faces, BSP_ACTIVE_TREE->face_count);
      }
    }
    else
    {
      gRendererState->active_cam = &editor_cam;
      EditorLoop(game_container.window, &gDrawQ, &editor_cam, input_state.mx, input_state.my, gConsole->visible);
    }

    // Draw split planes
    if (RENDERER_HASFLAG(gRendererState, RENDERER_FLAG_BSP_DRAWDEBUGPLANES))
    {
      R_DrawBSPPlanes();
    }

    R_DrawAABB(
        (Vector){gPlayers->x[0], gPlayers->y[0] + (gPlayers->aabbheight[0] * 0.5f), gPlayers->z[0]},
        VectorInit(gPlayers->aabbwidth[0] * 0.5f, gPlayers->aabbheight[0] * 0.5f, gPlayers->aabbwidth[0] * 0.5f),
        (Vector4){0, 1, 0, 1});

    if (!BSP_IsSolid(BSP_ACTIVE_TREE, (Vector){gPlayers->x[0], gPlayers->y[0], gPlayers->z[0]}))
    {
      // printf("EMPTY SPACE\n");
    }
    else
    {
      // printf("   SOLID SPACE\n");
    }
    

    ConsoleGUI_Draw();
    RDrawQueue_Execute(&gDrawQ);
    R_DrawTextBatches(game_container.win_w, game_container.win_h);
    ImGui_Render();

    SDL_GL_SwapWindow(game_container.window);
  }
  // RigidbodyArray_Destroy(&rb_arr);
  // ColliderArray_Destroy(&collider_arr);
  // MeshPrimitives_Destroy();
  // MeshDestroy(tri_mesh);
  // free(tri_mesh);
  // EditorDestroy(editor_state);
  Renderer_Destroy(gRendererState);
  MEM_ARENA_DESTROY(gMemArena);
  ApplicationDestroy(&game_container);

  return 0;
}
