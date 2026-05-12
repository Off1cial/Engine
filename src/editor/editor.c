#include "editor/editor.h"
#include "editor/editorcmd.h"
#include <float.h>
#include "rendering/render_commands.h"
#include "rendering/renderer.h"
#include "rendering/frustum.h"
#include "types/types_base.h"
#include "editor/brush.h"

editor_cmd_queue_t* gEditorQueue = NULL;









void EditorInit(editor_state_t* eState, SDL_Window* window, SDL_GLContext glContext){
  

  EditorBrushArray_Init(gEditorBrushArray, 32);
  printf("[EDITOR]: Brush array initialised\n");
  EditorGui_Init(window, glContext, eState->camera);
  printf("[EDITOR: EditorGUI initialised\n");

  editor_cmd_queue_t* q = malloc(sizeof(editor_cmd_queue_t));
  EditorQueue_Init(q, 32);
  gEditorQueue = q;
  
}

void EditorDestroy(editor_state_t* eState){
  //EditorBrushArray_Destroy(gEditorBrushArray);
  free(eState->b_arr);
  free(eState);
}

void EditorToggle(SDL_Window* window){
  *gEditorActive = !(*gEditorActive);
  

  int winw, winh;
  SDL_GetWindowSizeInPixels(window, &winw, &winh);
  if (*gEditorActive){
    Camera_Switch(1, winh);
    RecalculatePanels(winw, winh, gCameras[1]);
  }else{
    Camera_Switch(0, winh);
  }
}

void find_hovered_brush(
  camera_t* camera,
  float mx,
  float my,
  Vector* out_hit,
  float* out_dist,
  int* out_side)
{
  bool hit = false;

  for (size_t b = 0; b < gEditorBrushArray->count; b++){
    brush_t* brush = &gEditorBrushArray->brushes[b];

    bool ray_hit = Brush_Raycast(
      brush,
      out_side,
      out_hit,
      out_dist,
      camera,
      mx,
      my
    );

    if (!ray_hit){
      continue;
    }

    hit = true;

    brush_side_t* hovered_side =
      &brush->sides[*out_side];

    if (hovered_side != gEditorBrushArray->hovered_side.side){
      gEditorBrushArray->hovered_side.dirty = 1;
      gEditorBrushArray->hovered_side.side = hovered_side;
      gEditorBrushArray->hovered_side.owner_brush = brush;
    }

    break;
  }

  if (!hit){
    gEditorBrushArray->hovered_side.side = NULL;
    gEditorBrushArray->hovered_side.owner_brush = NULL;
    gEditorBrushArray->hovered_side.dirty = 1;
  }
}

void EditorLoop(SDL_Window* window, rdrawqueue_t* draw_q, camera_t* editor_camera, float mx, float my){

  bool relative_mouse = SDL_GetWindowRelativeMouseMode(window);

  Vector brush_hit = VECTOR_NAN;
  float brush_dist;
  int brush_hit_side;


  EditorQueue_Reset(gEditorQueue);

  // Handle editor camera input
  Vector mov_dir = VECTOR_ZERO;
  float fwd = 0.0f;
  float side = 0.0f;
  float up = 0.0f;
  if (gInputState->kCurrent[SDL_SCANCODE_W]) {fwd += 1.0f; }
  if (gInputState->kCurrent[SDL_SCANCODE_S]) {fwd -= 1.0f; }
  if (gInputState->kCurrent[SDL_SCANCODE_A]) {side -= 1.0f; }
  if (gInputState->kCurrent[SDL_SCANCODE_D]) {side += 1.0f; }
  if (gInputState->kCurrent[SDL_SCANCODE_SPACE]) {up += 1.0f; }
  if (gInputState->kCurrent[SDL_SCANCODE_LCTRL]) {up -= 1.0f; }

  Vector front = VectorScale(editor_camera->front, fwd);
  Vector right = VectorScale(editor_camera->right, side);
  Vector worldup = VectorScale(editor_camera->worldUp, up);

  mov_dir = VectorAdd(mov_dir, VectorAdd(front, right));
  mov_dir = VectorAdd(mov_dir, worldup);
  if (!VECTOR_IS_ZERO(mov_dir)) VectorNormalise(&mov_dir);


  Camera_Move(mov_dir, 0.5f, editor_camera);

  for (size_t i = 0; i < gEditorBrushArray->count; i++){
    EditorBrush_Draw(&gEditorBrushArray->brushes[i], gRendererState->draw_q, gRendererState->active_cam);
  }
  EditorBrush_DrawHoveredSide(&gEditorBrushArray->hovered_side);

  EditorGui_DrawAll(window, gInputState, editor_camera, gInputState->FLAG_WindowResized);
  EditorGui_HandleBrushInput(gInputState);
  EditorGui_HandlePanelInput(window, gInputState);
  if (!relative_mouse) {find_hovered_brush(editor_camera, mx, my, &brush_hit, &brush_dist, &brush_hit_side); }
  EditorQueue_Execute(gEditorQueue, gEditorBrushArray);
}
