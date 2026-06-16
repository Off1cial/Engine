#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>

#include "application.h"
#include "mem.h"

#include "editor/editor.h"
#include "editor/editorcmd.h"
#include "editor/bsp.h"
#include "editor/csg.h"

#include "inputbase.h"
#include "types/types_vector.h"

#include "rendering/camera.h"
#include "rendering/text.h"
#include "rendering/shader.h"
#include "rendering/mesh.h"
#include "rendering/render_commands.h"
#include "rendering/draw_list.h"
#include "rendering/renderer.h"

#include "console/console.h"
#include "console/consolegui.h"

#include "physics/physbody.h"
#include "player/player.h"
#include "player/pmove.h"

#include "state.h"

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------
struct mem_arena_t *gMemArena = NULL;
struct inputstate_t *gInputState = NULL;
renderer_state_t *gRendererState = NULL;
bool *gEditorActive = NULL;
editor_brush_array *gEditorBrushArray = NULL;
bsp_tree_t *BSP_ACTIVE_TREE = NULL;
bool gCursorLocked = false;

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
int main(void)
{
  const char *PATH_Assets = "../Assets/";
  int app_running;

  // ----- Application & OpenGL -----
  struct container_t game_container;
  APPLIACTION_CONTAINER = &game_container;
  app_running = !ApplicationInit(&game_container, 640, 480);
  SDL_GL_SetSwapInterval(1);

  // ----- Memory arena -----
  struct mem_arena_t mem_arena;
  gMemArena = &mem_arena;
  MEM_ARENA_INIT(gMemArena);
  MEM_ARENA_RESET(gMemArena);

  // ----- Input -----
  struct inputstate_t input_state;
  gInputState = &input_state;

  // ----- Shaders -----
  shader_store_t shader_store = {0};
  ShaderStore_Init(&shader_store, 16);

  shader_t *shader_unlit = malloc(sizeof(shader_t));
  shader_t *shader_lit = malloc(sizeof(shader_t));
  Shader_Load(shader_unlit, PATH_Assets, "Shaders/vert_unlit.vs", "Shaders/frag_unlit.fs");
  Shader_Load(shader_lit, PATH_Assets, "Shaders/vert_lit.vs", "Shaders/frag_lit.fs");

  // ----- Render queue -----
  rdrawqueue_t gDrawQ;
  RDrawQueue_Init(&gDrawQ, 128);

  // ----- Primitives -----
  MeshPrimitives_Init();

  // ----- Cameras -----
  camera_t main_cam;
  struct Viewport view = {
      .x = 0, .y = 0, .w = APPLIACTION_CONTAINER->win_w, .h = APPLIACTION_CONTAINER->win_h};
  Camera_init(&main_cam, VectorInit(0, 0, 0), view);
  gCameraIndex = 0;
  main_cam.sens = 0.1f;

  camera_t editor_cam;
  struct Viewport eView = {0, 0, 640, 480};
  Camera_init(&editor_cam, VectorInit(0, 10, 0), eView);
  editor_cam.sens = 0.04f;

  // ----- Renderer state -----
  renderer_state_t renderer_state = {0};
  gRendererState = &renderer_state;

  renderer_state.active_cam = gCameras[gCameraIndex];
  renderer_state.draw_q = &gDrawQ;
  renderer_state.shader_store = &shader_store;
  renderer_state.shader_unlit = renderer_state.shader_store->shaders[ShaderStore_Add(&shader_store, shader_unlit)];
  renderer_state.shader_lit = renderer_state.shader_store->shaders[ShaderStore_Add(&shader_store, shader_lit)];
  renderer_state.draw_normal_maps = true;
  renderer_state.wireframe = false;
  renderer_state.fullbright = false;

  renderer_state.light_count = 0;
  renderer_state.light_forward_count = 0;
  renderer_state.textbatches2d_count = 0;
  renderer_state.textbatches3d_count = 0;

  // ----- Lights -----
  light_t pointLight = {
      .type = LIGHT_POINT,
      .colour = VectorInit(0.55f, 0.55f, 0.5f),
      .direction = VectorInit(3, -1, 0),
      .intensity = 0.8f,
      .position = VectorInit(0, 560, 0),
      .radius = 60.0f,
  };

  light_t spotLight = {
      .type = LIGHT_SPOT,
      .colour = VectorInit(0.7f, 0.7f, 0.8f),
      .direction = main_cam.front,
      .intensity = 2.0f,
      .position = VectorInit(0, -8, 0),
      .radius = 200.0f,
      .cutoff = cosf(RAD(30.0f * 0.5f)),
  };

  renderer_state.lights_forward[renderer_state.light_forward_count++] = &pointLight;
  renderer_state.lights_forward[renderer_state.light_forward_count++] = &spotLight;

  // ----- Materials -----
  material_t *mat_marble = Material_Load("../Assets/Materials/brickmaterial.mat");
  if (!mat_marble)
  {
    printf("mat_marble = NULL\n");
    exit(1);
  }
  Renderer_AddMaterial(&renderer_state, mat_marble);
  Renderer_AddMaterial(&renderer_state, Material_Load("../Assets/Materials/brushhover.mat"));
  Renderer_AddMaterial(&renderer_state, Material_Load("../Assets/Materials/debugplane.mat"));
  Renderer_AddMaterial(&renderer_state, Material_Load("../Assets/Materials/Editor/skybox.mat"));

  // ----- Fonts -----
  TTF_Init();
  font_t *FONT_TF2BUILD = Font_Load("../Assets/Fonts/tf2build.ttf", 40);

  // ----- Editor -----
  Console_Init();
  editor_brush_array brush_array = {0};
  gEditorBrushArray = &brush_array;

  bool editor_active = false;
  gEditorActive = &editor_active;

  editor_state_t *editor_state = malloc(sizeof(editor_state_t));
  editor_state->camera = &editor_cam;
  EditorInit(editor_state, game_container.window, game_container.glContext);

  if (!gCameras[gCameraIndex])
    printf("Null camera\n");

  Camera_Switch(0, game_container.win_w, game_container.win_h);


 // ----- World -----
  
  
  EditorCreate_BrushRoom_CSG(gEditorBrushArray,
                         VectorInit(-150, 500, -150),
                         VectorInit(150, 800, 150), 0, 0);
    
  
  EditorCreate_BrushRoom_CSG(gEditorBrushArray,
                         VectorInit(-200, 0, -150),
                         VectorInit(100, 300, 150), 0, 0);
                         
  
  EditorCreate_BrushRoom(gEditorBrushArray,
                         VectorInit(-800, -900, -800),
                         VectorInit(800, 900, 800), 3, 1); // skybox

  

  /* TESTING THE CSG - IT WORKS
  
  float bscale = 100.0f;
  EditorCreate_Brush(gEditorBrushArray, VectorScale(VECTOR_ONE, -1), VECTOR_ONE, VectorScale(VECTOR_ONE, bscale), 0, 0);
  EditorCreate_Brush(gEditorBrushArray, VectorScale(VECTOR_ONE, -1), VECTOR_ONE, VectorScale(VECTOR_ONE, bscale * 0.5), 0, 0);
  brush_t* b0 = &gEditorBrushArray->brushes[0];
  brush_t* b1 = &gEditorBrushArray->brushes[1];
  Brush_Move(&gEditorBrushArray->brushes[1], VectorScale(VECTOR_ONE, bscale * 0.7f));
  gEditorBrushArray->brushes[1].dirty = 1;
  brush_t* csglist = NULL;
  int subtractioncount = CSG_SubtractBrush(&gEditorBrushArray->brushes[0], &gEditorBrushArray->brushes[1], &csglist);
  printf("[CSG: Subtraction count = %d\n", subtractioncount);
  for (int i = 0; i < subtractioncount; i++){
    csglist[i].dirty = 1;
    csglist->nodraw = 0;
    for (int j= 0; j < csglist[i].side_count; j++){
      csglist[i].sides[j].material_id = 1;
    }
    //Brush_Move(&csglist[i], VectorScale(VECTOR_AXIS_Y, 100.0f * (i + 1)));
    gEditorBrushArray->brushes[gEditorBrushArray->count++] = csglist[i];
  }
  b0->nodraw = 1;
  b1->nodraw = 1;

  */


  

  /* TESTING IF BRUSH SPLIT WORKS - IT DOES
  brush_t* b0 = &gEditorBrushArray->brushes[0];
  for (int i = 0; i < b0->side_count; i++){
    b0->sides[i].material_id = 0;
  }
  plane_t split = (plane_t){.dist = 0, .normal = VECTOR_AXIS_X};
  brush_t* f = malloc(sizeof(brush_t));
  brush_t* b = malloc(sizeof(brush_t));
  if (f == NULL){
    printf("f null\n");
  }
  if (b == NULL){
    printf("b null\n");
  }
  CSG_SplitBrush(b0, &split, &f, &b);
  if (f == NULL){
    printf("f null\n");
  }
  if (b == NULL){
    printf("b null\n");
  }
  Brush_Move(f, VectorScale(VECTOR_AXIS_Y, 50.0f));
  Brush_Move(b, VectorScale(VECTOR_AXIS_Y, -50.0f));
  printf("f sides = %d\n", f->side_count);
  printf("b sides = %d\n", b->side_count);
  f->dirty = 1;
  b->dirty =1;
  gEditorBrushArray->brushes[gEditorBrushArray->count++] = *f;
  gEditorBrushArray->brushes[gEditorBrushArray->count++] = *b;
  */
 
  // ----- Physics ------
  PhysbodyArray_Init(1);



  // ----- Player -----
  PlayerArrayInit(1);
  PlayerCreate(VectorInit(0, 570, 0), &main_cam);
;
  // ----- BSP -----
  BSP_ACTIVE_TREE = BSP2_Compile();

  // ----- Fixed timestep -----
  Uint64 lastTime = SDL_GetTicksNS();
  double accumulator = 0.0;
  const double FIXED_DT = 1.0 / 240.0;

  // =======================================================================
  // Main loop
  // =======================================================================
  while (app_running)
  {
    Uint64 now = SDL_GetTicksNS();
    double frameTime = (double)(now - lastTime) / 1e9;
    lastTime = now;

    if (frameTime > 0.25)
      frameTime = 0.25;
    accumulator += frameTime;

    poll_input(&input_state, &app_running,
      &game_container.win_w, &game_container.win_h,
      game_container.window);

    // ---- Fixed-step physics / game logic ----
    while (accumulator >= FIXED_DT)
    {
      // Update player logic (input → velocity)
      PlayerMove(0, 320, 160, 0, 0, 270, (float)FIXED_DT);
      

      // Integrate all physics bodies (including player)
      PhysbodyArray_Step((float)FIXED_DT, (Vector){0, -600, 0});
      PlayerUpdateGrounded(0);
      PlayerGroundClamp(0);

      PlayerCollideWorld(0);

      accumulator -= FIXED_DT;
    }

    // ---- Per-frame setup ----
    MEM_ARENA_RESET(gMemArena);
    RDrawQueue_Reset(&gDrawQ);


    // Hotkeys
    if (input_state.kCurrent[SDL_SCANCODE_K] && !input_state.kPrevious[SDL_SCANCODE_K])
      BSP_ACTIVE_TREE = BSP_Compile();

    if (input_state.FLAG_ToggleEditor)
      EditorToggle(game_container.window);

    // Update spotlight to follow player camera
    gRendererState->lights_forward[1]->position = main_cam.pos;
    gRendererState->lights_forward[1]->direction = main_cam.front;

    // ---- Clear ----
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ---- HUD text ----
    Text_AddString(FONT_TF2BUILD,
                   TextVector(PlayerPos(0)),
                   VectorInit(0, 0, 0),
                   (Vector4){1, 1, 1, 1}, true);

    // ---- ImGui ----
    ImGui_StartFrame();

    if (BSP_IsSolid(BSP_ACTIVE_TREE, gRendererState->active_cam->pos)) {
      printf("Camera in solid\n");
    }
    else {
      printf("EMPTY\n");
    }

    // ---- 3D scene ----
    if (!(*gEditorActive))
    {
      // Game mode
      if (input_state.FLAG_WindowResized)
        Camera_UpdateViewport(gPlayers->camera[0], 0, 0,
                              game_container.win_w, game_container.win_h,
                              game_container.win_h, 1);

      gRendererState->active_cam = gPlayers->camera[0];
      PlayerLook(0, gInputState->mx_rel, gInputState->my_rel);

      if (BSP_ACTIVE_TREE){
        R_DrawBSPFaces(&main_cam, BSP_ACTIVE_TREE->faces, BSP_ACTIVE_TREE->face_count);
      }
        
    }
    else
    {
      // Editor mode
      gRendererState->active_cam = &editor_cam;
      EditorLoop(game_container.window, &gDrawQ, &editor_cam,
                 input_state.mx, input_state.my, gConsole->visible);
    }

    // Debug planes
    if (RENDERER_HASFLAG(gRendererState, RENDERER_FLAG_BSP_DRAWDEBUGPLANES)){
      R_DrawBSPPlanes();
    } 
      

    // Player AABB
    int id = gPlayers->physbody_id[0];
    Vector center = {gPhysbodyArray->x[id], gPhysbodyArray->y[id], gPhysbodyArray->z[id]};
    Vector halfs = {gPhysbodyArray->halfx[id], gPhysbodyArray->halfy[id], gPhysbodyArray->halfz[id]};
    R_DrawAABB(center, halfs, (Vector4){0, 1, 0, 1});

    // ---- Flush rendering ----
    ConsoleGUI_Draw();
    RDrawQueue_Execute(&gDrawQ);
    R_DrawTextBatches(game_container.win_w, game_container.win_h);
    ImGui_Render();

    SDL_GL_SwapWindow(game_container.window);
  }

  // ----- Cleanup -----
  EditorBrushArray_Destroy(gEditorBrushArray);
  Renderer_Destroy(gRendererState);
  MEM_ARENA_DESTROY(gMemArena);
  ApplicationDestroy(&game_container);

  return 0;
}
