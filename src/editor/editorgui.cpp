#include "imgui_layer.h"
#include "editor/editorgui.h"
#include "editor/editorcamera.h"
#include "editor/editorgui_actions.h"
#include "editor/editorpanel.h"
#include "editor/editorcmd.h"
#include "rendering/camera.h"
#include "inputbase.h"
#include "imgui/imgui.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <cmath>

#define MAX_LABEL_LENGTH 32
#define GRID_SPACING_WORLD 10
#define PANEL_SPACING_PX (int)2

#define GRID_LINE_COL IM_COL32(200, 200, 200, 120)

bool start_up = true;
// Drawing brushes
ImVec2 brush_start;
ImVec2 brush_end;
bool drawing_brush = false;

int gEditorGui_HoveredPanel = 2;
brush_edge_t *edge_hovered = NULL;
brush_edge_t *edge_selected = NULL;
ImVec2 edge_hovered_a, edge_hovered_b;
ImVec2 edge_selected_a, edge_selected_b;
bool gEditorGui_ViewportCaptured = false;

bool dragging_edge;

ImVec2 imvec2_add(ImVec2 a, ImVec2 b)
{
  return ImVec2(a.x + b.x, a.y + b.y);
}

static ImVec2 Vector2_ImVec2(Vector2 v)
{
  return ImVec2(v.x, v.y);
}

gui_panel_t panels[6];

ImGuiWindowFlags panel_flags =
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoBringToFrontOnFocus |
    ImGuiWindowFlags_NoCollapse;

// Remove NoTitleBar

static void GetPanelBasis(
    int panel,
    Vector *right,
    Vector *up)
{
  switch (panel)
  {
  case PANEL_ORTHO_TOP: // TOP
    *right = VECTOR_AXIS_X;
    *up = VECTOR_AXIS_Z_NEG;
    break;

  case PANEL_ORTHO_SIDE: // SIDE
    *right = VECTOR_AXIS_Z;
    *up = VECTOR_AXIS_Y;
    break;

  case PANEL_ORTHO_FRONT: // FRONT
    *right = VECTOR_AXIS_X;
    *up = VECTOR_AXIS_Y;
    break;

  default:
    *right = VECTOR_ZERO;
    *up = VECTOR_ZERO;
    break;
  }
}

static ImVec2 ProjectToPanel(Vector v, Vector origin, Vector right, Vector up)
{
  return (ImVec2){
      (float)VectorDot(v, right),
      (float)VectorDot(v, up)};
}

void RecalculatePanels(int winw, int winh, camera_t *editor_camera)
{

  const int p0_w = (int)(winw / 4);

  const int top_bar_height = 60;

  // Width of the 4 render panels:
  int rwidth = (winw - p0_w) / 2;
  int rheight = (winh - top_bar_height) / 2;

  ImVec2 rsize = ImVec2(rwidth, rheight);

  // Main editor panel
  panels[PANEL_UI_MAIN] = {
      .pos = ImVec2(winw - p0_w, 0),
      .size = ImVec2(p0_w, winh),
      .label = "Main panel",
      .type = PANEL_TYPE_UI,
      .world_right = VECTOR_NAN,
      .world_up = VECTOR_NAN};
  // Top left - 3D
  panels[PANEL_VIEW_3D] = {
      .pos = ImVec2(0, top_bar_height),
      .size = rsize,
      .label = "3D",
      .type = PANEL_TYPE_VIEW,
      .world_right = gRendererState->active_cam->right,
      .world_up = gRendererState->active_cam->up,
  };
  // Top right  -TOP
  panels[PANEL_ORTHO_TOP] = {
      .pos = ImVec2(rwidth, top_bar_height),
      .size = rsize,
      .label = "TOP",
      .type = PANEL_TYPE_ORTHO,
      .world_right = VECTOR_AXIS_X,
      .world_up = VECTOR_AXIS_Z_NEG,

  };
  // Bottom right - SIDE
  panels[PANEL_ORTHO_SIDE] = {
      .pos = ImVec2(rwidth, rheight + top_bar_height),
      .size = rsize,
      .label = "SIDE",
      .type = PANEL_TYPE_ORTHO,
      .world_right = VECTOR_AXIS_Z,
      .world_up = VECTOR_AXIS_Y};
  // Bottom left - FRONT
  panels[PANEL_ORTHO_FRONT] = {
      .pos = ImVec2(0, rheight + top_bar_height),
      .size = rsize,
      .label = "FRONT",
      .type = PANEL_TYPE_ORTHO,
      .world_right = VECTOR_AXIS_X,
      .world_up = VECTOR_AXIS_Y};
  panels[PANEL_UI_TOOLS] = {
      .pos = ImVec2(0, 0),
      .size = ImVec2(winw - p0_w, top_bar_height),
      .label = "TOOLS",
      .type = PANEL_TYPE_UI,
      .world_right = VECTOR_NAN,
      .world_up = VECTOR_NAN};

  for (int i = 0; i < PANEL_COUNT - 1; i++)
  {
    panels[i].cam_pos = ImVec2(0, 0);
    panels[i].cam_zoom = 1.0f;
  }

  editor_camera->viewport = (struct Viewport){
      panels[PANEL_VIEW_3D].pos.x,
      panels[PANEL_VIEW_3D].pos.y,
      panels[PANEL_VIEW_3D].size.x,
      panels[PANEL_VIEW_3D].size.y};

  editor_camera->aspect = rsize.x / rsize.y;
  if (!start_up)

    glViewport(
        panels[PANEL_VIEW_3D].pos.x,
        winh - (panels[PANEL_VIEW_3D].pos.y + panels[PANEL_VIEW_3D].size.y),
        panels[PANEL_VIEW_3D].size.x,
        panels[PANEL_VIEW_3D].size.y);
}

void EditorGui_Init(SDL_Window *window, SDL_GLContext glContext, camera_t *editor_camera)
{
  ImGui_Init(window, glContext);
  int winw, winh;
  SDL_GetWindowSize(window, &winw, &winh);
  printf("Editor Window: %dx%d\n", winw, winh);
  RecalculatePanels(winw, winh, editor_camera);
  start_up = false;
}


static void draw_brushprogress_panellocal(ImVec2 a, ImVec2 b, int panel)
{
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 win_pos = ImGui::GetWindowPos();
    
    ImVec2 panel_a = a;
    ImVec2 panel_b = b;
    if (panel == PANEL_ORTHO_TOP){
      a.y = panels[panel].size.y - a.y;
      b.y = panels[panel].size.y - b.y;
    }
    // a and b are in panel-local coordinates (0,0 at top-left of panel)
    // Just add window position and draw
    ImVec2 min = ImVec2(
        win_pos.x + fmin(a.x, b.x),
        win_pos.y + fmin(a.y, b.y)
    );
    ImVec2 max = ImVec2(
        win_pos.x + fmax(a.x, b.x),
        win_pos.y + fmax(a.y, b.y)
    );

    dl->AddRect(min, max, IM_COL32(50, 180, 200, 200));
}

void draw_panel_grid(size_t index)
{

  if (panels[index].type != PANEL_TYPE_ORTHO)
  {
    return;
  }
  float half_w = panels[index].size.x / (2.0f * panels[index].cam_zoom);
  float half_h = panels[index].size.y / (2.0f * panels[index].cam_zoom);

  ImVec2 cpos = panels[index].cam_pos;

  float left = cpos.x - half_w + (PANEL_SPACING_PX / 2.0f);
  float right = cpos.x + half_w - (PANEL_SPACING_PX / 2.0f);

  float top = cpos.y + half_h - (PANEL_SPACING_PX / 2.0f);
  float bottom = cpos.y - half_h + (PANEL_SPACING_PX / 2.0f);

  float start_x = floorf(left / GRID_SPACING_WORLD) * GRID_SPACING_WORLD;
  float start_y = floorf(bottom / GRID_SPACING_WORLD) * GRID_SPACING_WORLD;

  ImDrawList *draw_list = ImGui::GetWindowDrawList();

  ImVec2 win_pos = ImGui::GetWindowPos();

  for (float x = start_x; x <= right; x += GRID_SPACING_WORLD)
  {

    ImVec2 scr_a = EditorCamera_WorldToScreen(panels[index].size, panels[index].cam_pos, panels[index].cam_zoom, ImVec2(x, bottom));
    ImVec2 scr_b = EditorCamera_WorldToScreen(panels[index].size, panels[index].cam_pos, panels[index].cam_zoom, ImVec2(x, top));

    scr_a = imvec2_add(scr_a, win_pos);
    scr_b = imvec2_add(scr_b, win_pos);
    // Draw lines
    //draw_list->AddLine(scr_a, scr_b, IM_COL32(255, 255, 255, 150), 0.8f);
    draw_list->AddLine(scr_a, scr_b, GRID_LINE_COL, 0.8f);
  }

  for (float y = start_y; y <= top; y += GRID_SPACING_WORLD)
  {

    ImVec2 scr_a =
        EditorCamera_WorldToScreen(panels[index].size, panels[index].cam_pos, panels[index].cam_zoom, ImVec2(left, y));
    ImVec2 scr_b =
        EditorCamera_WorldToScreen(panels[index].size, panels[index].cam_pos, panels[index].cam_zoom, ImVec2(right, y));

    scr_a = imvec2_add(scr_a, win_pos);
    scr_b = imvec2_add(scr_b, win_pos);
    //draw_list->AddLine(scr_a, scr_b, IM_COL32(255, 255, 255, 150), 0.8f);
    draw_list->AddLine(scr_a, scr_b, GRID_LINE_COL, 0.8f);
  }
}

static void draw_handle(brush_handle_t *handle, int panel)
{
  if (!handle)
    return;

  if (panels[panel].type != PANEL_TYPE_ORTHO)
    return;

  ImDrawList *dl = ImGui::GetWindowDrawList();
  ImVec2 win_pos = ImGui::GetWindowPos();

  switch (handle->type)
  {
  case BRUSH_HANDLE_SIDE:
    // Draw a selectable square on the line centre
    Vector right, up;
    // TOP VIEW
    if (panel == 2)
    {
      right = VECTOR_AXIS_X;
      up = VECTOR_AXIS_Z_NEG;
    }
    else if (panel == 3)
    {
      // SIDE
      right = VECTOR_AXIS_Z;
      up = VECTOR_AXIS_Y;
    }
    else if (panel == 4)
    {
      right = VECTOR_AXIS_X;
      up = VECTOR_AXIS_Y;
    }

    ImVec2 line_a = ProjectToPanel(
        handle->side_handle.edge->a,
        VECTOR_ZERO,
        right,
        up);

    ImVec2 line_b = ProjectToPanel(
        handle->side_handle.edge->b,
        VECTOR_ZERO,
        right,
        up);

    ImVec2 centre = ImVec2(line_a.x + line_b.x, line_a.y + line_b.y);
    centre.x *= 0.5f;
    centre.y *= 0.5f;

    centre.x += win_pos.x;
    centre.y += win_pos.y;

    const float rectsize_half = 4.0f;

    ImVec2 minp = ImVec2(centre.x - rectsize_half, centre.y + rectsize_half);
    ImVec2 maxp = ImVec2(centre.x + rectsize_half, centre.y + rectsize_half);

    dl->AddRectFilled(minp, maxp, IM_COL32(0, 255, 0, 255));
  }
}

static void draw_edge(ImDrawList *dl,
                      ImVec2 a, ImVec2 b,
                      ImVec2 offset)
{
  a.x += offset.x;
  a.y += offset.y;
  b.x += offset.x;
  b.y += offset.y;

  dl->AddLine(a, b, IM_COL32(200, 100, 0, 255), 1.0f);
}

static float point_segment_distance_sq(ImVec2 p, ImVec2 a, ImVec2 b)
{
  float abx = b.x - a.x;
  float aby = b.y - a.y;

  float apx = p.x - a.x;
  float apy = p.y - a.y;

  float ab_len_sq = abx * abx + aby * aby;

  float t = 0.0f;

  if (ab_len_sq > 0.0f)
  {
    t = (apx * abx + apy * aby) / ab_len_sq;
  }

  if (t < 0.0f)
    t = 0.0f;
  if (t > 1.0f)
    t = 1.0f;

  float cx = a.x + abx * t;
  float cy = a.y + aby * t;

  float dx = p.x - cx;
  float dy = p.y - cy;

  return dx * dx + dy * dy;
}

static brush_edge_t *find_hovered_edge(int panel)
{
  if (panels[panel].type != PANEL_TYPE_ORTHO)
    return NULL;

  Vector right, up;

  switch (panel)
  {
  case PANEL_ORTHO_TOP:
    right = VECTOR_AXIS_X;
    up = VECTOR_AXIS_Z_NEG;
    break;

  case PANEL_ORTHO_SIDE:
    right = VECTOR_AXIS_Z;
    up = VECTOR_AXIS_Y;
    break;

  case PANEL_ORTHO_FRONT:
    right = VECTOR_AXIS_X;
    up = VECTOR_AXIS_Y;
    break;

  default:
    return NULL;
  }

  ImVec2 mouse = ImGui::GetMousePos();

  brush_edge_t *best_edge = NULL;

  float best_dist = FLT_MAX;

  const float threshold = 6.0f;
  const float threshold_sq = threshold * threshold;

  for (size_t b = 0; b < gEditorBrushArray->count; b++)
  {
    brush_t *brush = &gEditorBrushArray->brushes[b];

    for (size_t e = 0; e < brush->edge_count; e++)
    {
      brush_edge_t *edge = &brush->edges[e];

      // WORLD → PANEL
      ImVec2 a2 = ProjectToPanel(
          edge->a,
          VECTOR_ZERO,
          right,
          up);

      ImVec2 b2 = ProjectToPanel(
          edge->b,
          VECTOR_ZERO,
          right,
          up);

      // PANEL → SCREEN
      ImVec2 sa = EditorCamera_WorldToScreen(
          panels[panel].size,
          panels[panel].cam_pos,
          panels[panel].cam_zoom,
          a2);

      ImVec2 sb = EditorCamera_WorldToScreen(
          panels[panel].size,
          panels[panel].cam_pos,
          panels[panel].cam_zoom,
          b2);

      // WINDOW OFFSET
      ImVec2 win_pos = ImGui::GetWindowPos();

      sa.x += win_pos.x;
      sa.y += win_pos.y;

      sb.x += win_pos.x;
      sb.y += win_pos.y;

      float dist_sq =
          point_segment_distance_sq(mouse, sa, sb);

      if (dist_sq < threshold_sq &&
          dist_sq < best_dist)
      {
        best_dist = dist_sq;
        best_edge = edge;
        edge_hovered_a = sa;
        edge_hovered_b = sb;
      }
    }
  }

  return best_edge;
}

static void draw_brush_edges(brush_t *brush, int panel)
{
  if (panels[panel].type != PANEL_TYPE_ORTHO)
    return;

  Vector right, up;

  switch (panel)
  {
  case PANEL_ORTHO_TOP: // TOP (X/Z)
    right = VECTOR_AXIS_X;
    up = VECTOR_AXIS_Z_NEG;
    break;

  case PANEL_ORTHO_SIDE: // SIDE (Z/Y)
    right = VECTOR_AXIS_Z;
    up = VECTOR_AXIS_Y;
    break;

  case PANEL_ORTHO_FRONT: // FRONT (X/Y)
    right = VECTOR_AXIS_X;
    up = VECTOR_AXIS_Y;
    break;

  default:
    return;
  }

  ImDrawList *dl = ImGui::GetWindowDrawList();
  ImVec2 win_pos = ImGui::GetWindowPos();

  for (size_t e = 0; e < brush->edge_count; e++)
  {
    brush_edge_t *edge = &brush->edges[e];

    // WORLD → PANEL SPACE
    ImVec2 a2 = ProjectToPanel(
        edge->a,
        VECTOR_ZERO,
        right,
        up);

    ImVec2 b2 = ProjectToPanel(
        edge->b,
        VECTOR_ZERO,
        right,
        up);

    // PANEL SPACE → SCREEN SPACE
    ImVec2 sa = EditorCamera_WorldToScreen(
        panels[panel].size,
        panels[panel].cam_pos,
        panels[panel].cam_zoom,
        a2);

    ImVec2 sb = EditorCamera_WorldToScreen(
        panels[panel].size,
        panels[panel].cam_pos,
        panels[panel].cam_zoom,
        b2);

    // WINDOW OFFSET
    sa.x += win_pos.x;
    sa.y += win_pos.y;

    sb.x += win_pos.x;
    sb.y += win_pos.y;

    dl->AddLine(
        sa,
        sb,
        IM_COL32(200, 100, 0, 255),
        1.0f);
  }
}

void draw_brush_planes(brush_t *brush, int panel)
{
  if ((panels[panel].type != PANEL_TYPE_ORTHO) || !brush)
    return;

  mesh_t *mesh = &brush->editor_mesh;

  // ----------------------------
  // Panel basis (replace later with stored camera basis)
  // ----------------------------
  Vector origin = brush->pos;

  Vector right, up;

  switch (panel)
  {
  case PANEL_ORTHO_TOP: // TOP (X/Z)
    right = (Vector){1, 0, 0};
    up = (Vector){0, 0, -1};
    break;

  case PANEL_ORTHO_SIDE: // SIDE (Z/Y)
    right = (Vector){0, 0, 1};
    up = (Vector){0, 1, 0};
    break;

  case PANEL_ORTHO_FRONT: // FRONT (X/Y)
    right = (Vector){1, 0, 0};
    up = (Vector){0, 1, 0};
    break;

  default:
    return;
  }

  ImDrawList *dl = ImGui::GetWindowDrawList();
  ImVec2 win_pos = ImGui::GetWindowPos();

  // ----------------------------
  // TRIANGLE RENDER (no fake edges)
  // ----------------------------
  for (size_t i = 0; i + 2 < mesh->index_count; i += 3)
  {
    uint32_t i0 = mesh->indices[i + 0];
    uint32_t i1 = mesh->indices[i + 1];
    uint32_t i2 = mesh->indices[i + 2];

    Vector v0 = mesh->vertices[i0].pos;
    Vector v1 = mesh->vertices[i1].pos;
    Vector v2 = mesh->vertices[i2].pos;

    // Project to panel space
    ImVec2 p0 = ProjectToPanel(v0, origin, right, up);
    ImVec2 p1 = ProjectToPanel(v1, origin, right, up);
    ImVec2 p2 = ProjectToPanel(v2, origin, right, up);

    // Convert to screen space
    ImVec2 s0 = EditorCamera_WorldToScreen(
        panels[panel].size,
        panels[panel].cam_pos,
        panels[panel].cam_zoom,
        p0);

    ImVec2 s1 = EditorCamera_WorldToScreen(
        panels[panel].size,
        panels[panel].cam_pos,
        panels[panel].cam_zoom,
        p1);

    ImVec2 s2 = EditorCamera_WorldToScreen(
        panels[panel].size,
        panels[panel].cam_pos,
        panels[panel].cam_zoom,
        p2);

    // offset into ImGui window
    s0.x += win_pos.x;
    s0.y += win_pos.y;
    s1.x += win_pos.x;
    s1.y += win_pos.y;
    s2.x += win_pos.x;
    s2.y += win_pos.y;

    // draw triangle wireframe
    dl->AddLine(s0, s1, IM_COL32(200, 100, 0, 255), 1.0f);
    dl->AddLine(s1, s2, IM_COL32(200, 100, 0, 255), 1.0f);
    dl->AddLine(s2, s0, IM_COL32(200, 100, 0, 255), 1.0f);
  }
}

void draw_tool_panel_contents(){
  // Sprites for each tool, selectable buttons that change tool
}

void draw_main_panel_contents()
{
}

void draw_panel(struct inputstate_t *input, size_t index, int winw, int winh, camera_t* editor_camera)
{

  ImGui::SetNextWindowPos(panels[index].pos);
  ImGui::SetNextWindowSize(panels[index].size);

  ImGuiWindowFlags flags;

  if (panels[index].type == PANEL_TYPE_VIEW)
  {
    flags = panel_flags | ImGuiWindowFlags_NoBackground;
  }
  else
  {
    flags = panel_flags;
  }

  ImGui::Begin(panels[index].label, nullptr, flags);

  if (ImGui::IsWindowHovered())
  {
    gEditorGui_HoveredPanel = index;
  }

  draw_panel_grid(index);
  if (index == gEditorGui_HoveredPanel)
    edge_hovered = find_hovered_edge(index);

  for (size_t b = 0; b < gEditorBrushArray->count; b++)
  {
    brush_t *brush = &gEditorBrushArray->brushes[b];
    draw_brush_edges(brush, index);
  }

  if (drawing_brush && (index == gEditorGui_HoveredPanel)){
    draw_brushprogress_panellocal(brush_start, brush_end, index);
  }


  if (edge_hovered)
  {
    ImGui::GetWindowDrawList()->AddLine(
        edge_hovered_a,
        edge_hovered_b,
        IM_COL32(0, 200, 30, 255));
  }

  if (edge_selected)
  {
    ImGui::GetWindowDrawList()->AddLine(
        edge_selected_a,
        edge_selected_b,
        IM_COL32(0, 200, 180, 255));
  }

  ImGui::End();
}

void EditorGui_DrawAll(SDL_Window *window, struct inputstate_t *input, camera_t *editor_camera, bool resize_flag)
{
  int winw, winh;
  if (resize_flag)
  {
    
    SDL_GetWindowSize(window, &winw, &winh);
    RecalculatePanels(winw, winh, editor_camera);
  }

  // ImGui_StartFrame();

  for (int i = 0; i < 6; i++)
  {
    
    draw_panel(input, i, winw, winh, editor_camera);
  }

  // ImGui_Render();
}

static Vector PanelMouseToWorldDelta(Vector right, Vector up, float mx, float my) // Useful for dragging brush posiitons
{
  return VectorAdd(
      VectorScale(right, mx),
      VectorScale(up, -my));
}


static Vector ImVec2ToWorld(int panel, ImVec2 vec){
  ImVec2 wvec2 = EditorCamera_ScreenToWorld(
    panels[panel].size,
    panels[panel].cam_pos,
    panels[panel].cam_zoom,
    vec
  );

  Vector world = {0, 0, 0};

  switch(panel){
    case PANEL_ORTHO_TOP:
      world = (Vector){wvec2.x, 0, wvec2.y};
      break;
    case PANEL_ORTHO_SIDE:
      world = (Vector){0, wvec2.y, wvec2.x};
      break;
    case PANEL_ORTHO_FRONT:
      world = (Vector){wvec2.x, wvec2.y, 0};
      break;
    default:
      break;
  }
  return world;
}



static Vector draw_start_world;
void EditorGui_HandleBrushInput(struct inputstate_t *input)
{
  static bool drawing = false;
  static ImVec2 start;

  Vector up, right;

  GetPanelBasis(gEditorGui_HoveredPanel, &right, &up);

  if (panels[gEditorGui_HoveredPanel].type != PANEL_TYPE_ORTHO)
    return;

  // bool isTopPanel = (gEditorGui_HoveredPanel == PANEL_ORTHO_TOP);
  // bool isTopPanel = true;

  // Begin drawing
  if (input->mbutton_left_toggle && !drawing && !edge_hovered && !edge_selected)
  {
    printf("Began drawing\n");
    drawing = true;
    drawing_brush = drawing;

    start = ImGui::GetMousePos();

    start.x -= panels[gEditorGui_HoveredPanel].pos.x;
    start.y -= panels[gEditorGui_HoveredPanel].pos.y;

    if (gEditorGui_HoveredPanel == PANEL_ORTHO_TOP)
    {
      // Flip Y for top panel: screen down -> world -Z
      start.y = panels[PANEL_ORTHO_TOP].size.y - start.y;
    }

    // start.y = (isTopPanel) ? panels[PANEL_ORTHO_TOP].size.y - start.y : start.y - panels[gEditorGui_HoveredPanel].pos.y;

    
  }
  // End drawing
  else if (drawing && !edge_hovered)
  {
    ImVec2 end = ImGui::GetMousePos();
    end.x -= panels[gEditorGui_HoveredPanel].pos.x;
    end.y -= panels[gEditorGui_HoveredPanel].pos.y;

    // end.y = (isTopPanel) ? panels[PANEL_ORTHO_TOP].size.y - end.y : end.y - panels[gEditorGui_HoveredPanel].pos.y;
    if (gEditorGui_HoveredPanel == PANEL_ORTHO_TOP)
    {
      // Flip Y for top panel: screen down -> world -Z
      //start.y = panels[PANEL_ORTHO_TOP].size.y - start.y;
      end.y = panels[PANEL_ORTHO_TOP].size.y - end.y;
    }
    brush_start = start;
    brush_end = end;
    if (input->mbutton_left_toggle)
    // Finish drawing
    {
      drawing = false;
      drawing_brush = drawing;
      panel_finalise_brush(
          gEditorGui_HoveredPanel,
          panels[gEditorGui_HoveredPanel].size,
          start,
          end,
          panels[gEditorGui_HoveredPanel].cam_pos,
          panels[gEditorGui_HoveredPanel].cam_zoom,
          GRID_SPACING_WORLD);
    }
  }

  // Edge selection
  if (input->mbutton_left_toggle && edge_hovered)
  {
    printf("Edge selected\n");
    edge_selected = edge_hovered;

    edge_selected_a = edge_hovered_a;
    edge_selected_b = edge_hovered_b;
  }
  else if (input->mbutton_left_toggle && !drawing && !edge_hovered)
  {
    edge_selected = NULL;
  }
  if (input->mCurrent[SDL_BUTTON_LEFT] && edge_selected)
  {
    brush_t *brush = &gEditorBrushArray->brushes[edge_selected->brush];
    brush_side_t *a = &brush->sides[edge_selected->side_a];
    brush_side_t *b = &brush->sides[edge_selected->side_b];

    // 1. Convert screen pixel delta to panel-space (world) delta
    float inv_zoom = 1.0f / panels[gEditorGui_HoveredPanel].cam_zoom;
    float panel_mx = input->mx_rel * inv_zoom;
    float panel_my = input->my_rel * inv_zoom;

    Vector world_delta = VectorAdd(
        VectorScale(right, panel_mx),
        VectorScale(up, -panel_my));

    // 2. Modify the plane whose normal is most aligned with the drag
    float dot_a = VectorDot(a->plane.normal, world_delta);
    float dot_b = VectorDot(b->plane.normal, world_delta);

    const float eps = 1e-5f;
    if (fabsf(dot_a) > fabsf(dot_b) && fabsf(dot_a) > eps)
    {
      a->plane.dist += dot_a;
      brush->dirty = 1;
      brush->dirty_ui_edges = 1;
    }
    else if (fabsf(dot_b) > eps)
    {
      b->plane.dist += dot_b;
      brush->dirty = 1;
      brush->dirty_ui_edges = 1;
    }
  }
}

void EditorGui_HandlePanelInput(SDL_Window *window, struct inputstate_t *input)
{

  // Handle zoom
  panels[gEditorGui_HoveredPanel].cam_zoom += input->scrl_y * 0.1f;

  float *cam_zoom = &panels[gEditorGui_HoveredPanel].cam_zoom;

  if (*cam_zoom < 0.5f)
    *cam_zoom = 0.5f;
  if (*cam_zoom > 5.0f)
    *cam_zoom = 5.0f;

  // Handle panning
  if ((panels[gEditorGui_HoveredPanel].type == PANEL_TYPE_ORTHO) && input->mbutton_right)
  {
    panels[gEditorGui_HoveredPanel].cam_pos.x -= input->mx_rel / (*cam_zoom);
    panels[gEditorGui_HoveredPanel].cam_pos.y += input->my_rel / (*cam_zoom);
  }

  // Handle 3D panel relativity
  if ((panels[gEditorGui_HoveredPanel].type == PANEL_TYPE_VIEW) && input->mbutton_left_toggle)
  {
    gEditorGui_ViewportCaptured = true;
    gCursorLocked = true;
    //SDL_SetWindowRelativeMouseMode(window, true);
  }
  if (gEditorGui_ViewportCaptured &&
      input->kCurrent[SDL_SCANCODE_ESCAPE] &&
      !input->kPrevious[SDL_SCANCODE_ESCAPE])
  {
    gEditorGui_ViewportCaptured = false;
    gCursorLocked = false;
    //SDL_SetWindowRelativeMouseMode(window, false);
  }
  if (gEditorGui_ViewportCaptured)
  {
    Camera_Look(gCameras[gCameraIndex], input->mx_rel, input->my_rel);
  }
}
