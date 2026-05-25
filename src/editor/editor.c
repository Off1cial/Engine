#include "editor/editor.h"
#include "editor/editorcmd.h"
#include <float.h>
#include "rendering/render_commands.h"
#include "rendering/renderer.h"
#include "rendering/frustum.h"
#include "types/types_base.h"
#include "editor/brush.h"

editor_cmd_queue_t *gEditorQueue = NULL;

plane_drag_t *gEditorPlaneDrag = NULL;

void BeginPlaneDrag(float mx, float my)
{
  gEditorPlaneDrag->active = true;

  brush_side_t *side = gEditorBrushArray->hovered_side.side;

  gEditorPlaneDrag->brush = gEditorBrushArray->hovered_side.owner_brush;
  gEditorPlaneDrag->side = side;

  gEditorPlaneDrag->normal = side->plane.normal;
  gEditorPlaneDrag->initial_dist = side->plane.dist;

  gEditorPlaneDrag->start_mouse_x = mx;
  gEditorPlaneDrag->start_mouse_y = my;
}

void UpdatePlaneDrag(float mx, float my)
{
  if (!gEditorPlaneDrag->active)
    return;

  float dx = floorf(mx - gEditorPlaneDrag->start_mouse_x);
  float dy = floorf(my - gEditorPlaneDrag->start_mouse_y);

  float sensitivity = 1.0f;

  // camera-facing basis (simple + stable)
  camera_t *cam = gRendererState->active_cam;

  Vector right = cam->right;
  Vector up = cam->worldUp;

  Vector move = VectorAdd(
      VectorScale(right, dx * sensitivity),
      VectorScale(up, -dy * sensitivity));

  float move_amount =
      (int)VectorDot(move, gEditorPlaneDrag->normal) / 5;

  gEditorPlaneDrag->side->plane.dist =
      gEditorPlaneDrag->initial_dist + move_amount;

  gEditorPlaneDrag->brush->dirty = 1;
}

void EditorInit(editor_state_t *eState, SDL_Window *window, SDL_GLContext glContext)
{
  gEditorPlaneDrag = malloc(sizeof(plane_drag_t));
  memset(gEditorPlaneDrag, 0, sizeof(plane_drag_t));

  EditorBrushArray_Init(gEditorBrushArray, 32);
  printf("[EDITOR]: Brush array initialised\n");
  EditorGui_Init(window, glContext, eState->camera);
  printf("[EDITOR: EditorGUI initialised\n");

  editor_cmd_queue_t *q = malloc(sizeof(editor_cmd_queue_t));
  EditorQueue_Init(q, 32);
  gEditorQueue = q;
}

void EditorDestroy(editor_state_t *eState)
{
  // EditorBrushArray_Destroy(gEditorBrushArray);
  free(eState->b_arr);
  free(eState);
}

void EditorToggle(SDL_Window *window)
{
  *gEditorActive = !(*gEditorActive);

  int winw, winh;
  SDL_GetWindowSizeInPixels(window, &winw, &winh);
  if (*gEditorActive)
  {
    Camera_Switch(1, winh);
    RecalculatePanels(winw, winh, gCameras[1]);
  }
  else
  {
    Camera_Switch(0, winh);
  }
}

void find_hovered_brush(
    camera_t *camera,
    float mx,
    float my,
    Vector *out_hit,
    float *out_dist,
    int *out_side)
{
  bool hit = false;

  for (size_t b = 0; b < gEditorBrushArray->count; b++)
  {
    brush_t *brush = &gEditorBrushArray->brushes[b];

    bool ray_hit = Brush_Raycast(
        brush,
        out_side,
        out_hit,
        out_dist,
        camera,
        mx,
        my);

    if (!ray_hit)
    {
      continue;
    }

    hit = true;

    brush_side_t *hovered_side =
        &brush->sides[*out_side];

    if (hovered_side != gEditorBrushArray->hovered_side.side)
    {
      gEditorBrushArray->hovered_side.dirty = 1;
      gEditorBrushArray->hovered_side.side = hovered_side;
      gEditorBrushArray->hovered_side.owner_brush = brush;
    }

    break;
  }

  if (!hit)
  {
    gEditorBrushArray->hovered_side.side = NULL;
    gEditorBrushArray->hovered_side.owner_brush = NULL;
    gEditorBrushArray->hovered_side.dirty = 1;
  }
}

void EditorLoop(SDL_Window *window, rdrawqueue_t *draw_q, camera_t *editor_camera, float mx, float my, int console_open)
{

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
  if (gInputState->kCurrent[SDL_SCANCODE_W])
  {
    fwd += 1.0f;
  }
  if (gInputState->kCurrent[SDL_SCANCODE_S])
  {
    fwd -= 1.0f;
  }
  if (gInputState->kCurrent[SDL_SCANCODE_A])
  {
    side -= 1.0f;
  }
  if (gInputState->kCurrent[SDL_SCANCODE_D])
  {
    side += 1.0f;
  }
  if (gInputState->kCurrent[SDL_SCANCODE_SPACE])
  {
    up += 1.0f;
  }
  if (gInputState->kCurrent[SDL_SCANCODE_LCTRL])
  {
    up -= 1.0f;
  }

  Vector front = VectorScale(editor_camera->front, fwd);
  Vector right = VectorScale(editor_camera->right, side);
  Vector worldup = VectorScale(editor_camera->worldUp, up);

  mov_dir = VectorAdd(mov_dir, VectorAdd(front, right));
  mov_dir = VectorAdd(mov_dir, worldup);
  if (!VECTOR_IS_ZERO(mov_dir))
    VectorNormalise(&mov_dir);

  Camera_Move(mov_dir, 0.5f, editor_camera);

  for (size_t i = 0; i < gEditorBrushArray->count; i++)
  {
    EditorBrush_Draw(&gEditorBrushArray->brushes[i], gRendererState->draw_q, gRendererState->active_cam);
  }
  EditorBrush_DrawHoveredSide(&gEditorBrushArray->hovered_side, 0);

  EditorGui_DrawAll(window, gInputState, editor_camera, gInputState->FLAG_WindowResized);
  
  if (!console_open){
    EditorGui_HandleBrushInput(gInputState);
    EditorGui_HandlePanelInput(window, gInputState);
    if (!relative_mouse)
    {
      find_hovered_brush(editor_camera, mx, my, &brush_hit, &brush_dist, &brush_hit_side);
    }

    // ---------------------------------------------------------------
    // Testing 3d brush dragging
    if (gInputState->mbutton_right_toggle && gEditorBrushArray->hovered_side.side)
    {
      // relative_mouse = false;
      BeginPlaneDrag(gInputState->mx, gInputState->my);
    }

    if (gInputState->mbutton_right && gEditorPlaneDrag->active)
    {
      // relative_mouse = false;
      UpdatePlaneDrag(gInputState->mx, gInputState->my);
    }

    if (gInputState->mbutton_right_released)
    {
      gEditorPlaneDrag->active = false;
    }
    // ----------------------------------------------------------------
  }



  EditorQueue_Execute(gEditorQueue, gEditorBrushArray);
}
