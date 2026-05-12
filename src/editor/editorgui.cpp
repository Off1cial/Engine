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

ImVec2 imvec2_add(ImVec2 a, ImVec2 b)
{
  return ImVec2(a.x + b.x, a.y + b.y);
}

static ImVec2 Vector2_ImVec2(Vector2 v)
{
  return ImVec2(v.x, v.y);
}

gui_panel_t panels[5];

ImGuiWindowFlags panel_flags =
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoBringToFrontOnFocus |
    ImGuiWindowFlags_NoCollapse |
    ImGuiWindowFlags_NoTitleBar;
// Remove NoTitleBar

void RecalculatePanels(int winw, int winh, camera_t *editor_camera)
{

  const int p0_w = (int)(winw / 3);

  // Width of the 4 render panels:
  int rwidth = (winw - p0_w) / 2;
  int rheight = (winh / 2);

  ImVec2 rsize = ImVec2(rwidth, rheight);

  // Main editor panel
  panels[0] = {
      .pos = ImVec2(winw - p0_w, 0),
      .size = ImVec2(p0_w, winh),
      .label = "Main panel"};
  // Top left - 3D
  panels[1] = {
      .pos = ImVec2(0, 0),
      .size = rsize,
      .label = "3D"};
  // Top right  -TOP
  panels[2] = {
      .pos = ImVec2(rwidth, 0),
      .size = rsize,
      .label = "TOP"};
  // Bottom right - SIDE
  panels[3] = {
      .pos = ImVec2(rwidth, rheight),
      .size = rsize,
      .label = "SIDE"};
  // Bottom left - FRONT
  panels[4] = {
      .pos = ImVec2(0, rheight),
      .size = rsize,
      .label = "FRONT"};

  for (int i = 0; i < 5; i++)
  {
    panels[i].cam_pos = ImVec2(0, 0);
    panels[i].cam_zoom = 1.0f;
  }

  editor_camera->viewport = (struct Viewport){0, 0, rsize.x, rsize.y};
  editor_camera->aspect = rsize.x / rsize.y;
  if (!start_up)
    glViewport(0, winh - rsize.y, rsize.x, rsize.y);
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

void draw_panel_grid(size_t index)
{

  if (index <= 1)
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
    draw_list->AddLine(scr_a, scr_b, IM_COL32(255, 255, 255, 150), 0.8f);
  }

  for (float y = start_y; y <= top; y += GRID_SPACING_WORLD)
  {

    ImVec2 scr_a =
        EditorCamera_WorldToScreen(panels[index].size, panels[index].cam_pos, panels[index].cam_zoom, ImVec2(left, y));
    ImVec2 scr_b =
        EditorCamera_WorldToScreen(panels[index].size, panels[index].cam_pos, panels[index].cam_zoom, ImVec2(right, y));

    scr_a = imvec2_add(scr_a, win_pos);
    scr_b = imvec2_add(scr_b, win_pos);
    draw_list->AddLine(scr_a, scr_b, IM_COL32(255, 255, 255, 150), 0.8f);
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

static ImVec2 ProjectToPanel(Vector v, Vector origin, Vector right, Vector up)
{
  return (ImVec2){
      (float)VectorDot(v, right),
      (float)VectorDot(v, up)};
}

static brush_edge_t *find_hovered_edge(int panel)
{
  if (panel < 2 || panel > 4)
    return NULL;

  Vector right, up;

  switch (panel)
  {
  case 2:
    right = VECTOR_AXIS_X;
    up = VECTOR_AXIS_Z_NEG;
    break;

  case 3:
    right = VECTOR_AXIS_Z;
    up = VECTOR_AXIS_Y;
    break;

  case 4:
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
  if (panel < 2 || panel > 4)
    return;

  Vector right, up;

  switch (panel)
  {
  case 2: // TOP (X/Z)
    right = VECTOR_AXIS_X;
    up = VECTOR_AXIS_Z_NEG;
    break;

  case 3: // SIDE (Z/Y)
    right = VECTOR_AXIS_Z;
    up = VECTOR_AXIS_Y;
    break;

  case 4: // FRONT (X/Y)
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
  if (panel <= 1 || !brush)
    return;

  mesh_t *mesh = &brush->editor_mesh;

  // ----------------------------
  // Panel basis (replace later with stored camera basis)
  // ----------------------------
  Vector origin = brush->pos;

  Vector right, up;

  switch (panel)
  {
  case 2: // TOP (X/Z)
    right = (Vector){1, 0, 0};
    up = (Vector){0, 0, -1};
    break;

  case 3: // SIDE (Z/Y)
    right = (Vector){0, 0, 1};
    up = (Vector){0, 1, 0};
    break;

  case 4: // FRONT (X/Y)
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

/*
void draw_brush_2d(int panel, size_t brush)
{
  if (panel <= 1)
    return;

  Vector bpos = gEditorBrushArray->brushes[brush].pos;

  Vector scale_half = VectorInit(
      gEditorBrushArray->brushes[brush].scale.x * 0.5f,
      gEditorBrushArray->brushes[brush].scale.y * 0.5f,
      gEditorBrushArray->brushes[brush].scale.z * 0.5f);

  Vector min = VectorSub(bpos, scale_half);
  Vector max = VectorAdd(bpos, scale_half);

  ImVec2 wrld_a, wrld_b;

  switch (panel)
  {
  case 2:
    wrld_a = ImVec2(min.x, min.z);
    wrld_b = ImVec2(max.x, max.z);
    break;

  case 3:
    wrld_a = ImVec2(min.z, min.y);
    wrld_b = ImVec2(max.z, max.y);
    break;

  case 4:
    wrld_a = ImVec2(min.x, min.y);
    wrld_b = ImVec2(max.x, max.y);
    break;

  default:
    return;
  }

  ImVec2 scr_a = EditorCamera_WorldToScreen(
      panels[panel].size,
      panels[panel].cam_pos,
      panels[panel].cam_zoom,
      wrld_a);

  ImVec2 scr_b = EditorCamera_WorldToScreen(
      panels[panel].size,
      panels[panel].cam_pos,
      panels[panel].cam_zoom,
      wrld_b);

  ImVec2 win_pos = ImGui::GetWindowPos();

  scr_a = imvec2_add(scr_a, win_pos);
  scr_b = imvec2_add(scr_b, win_pos);

  ImVec2 tl(fmin(scr_a.x, scr_b.x), fmin(scr_a.y, scr_b.y));
  ImVec2 br(fmax(scr_a.x, scr_b.x), fmax(scr_a.y, scr_b.y));

  ImGui::GetWindowDrawList()->AddRect(
      tl,
      br,
      IM_COL32(200, 100, 0, 255));
}
*/

void draw_main_panel_contents()
{
}

void draw_panel(struct inputstate_t *input, size_t index)
{

  ImGui::SetNextWindowPos(panels[index].pos);
  ImGui::SetNextWindowSize(panels[index].size);

  ImGuiWindowFlags flags;

  if (index == 1)
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
  edge_hovered = find_hovered_edge(index);

  for (size_t b = 0; b < gEditorBrushArray->count; b++)
  {
    brush_t *brush = &gEditorBrushArray->brushes[b];
    draw_brush_edges(brush, index);
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

  if (resize_flag)
  {
    int winw, winh;
    SDL_GetWindowSize(window, &winw, &winh);
    RecalculatePanels(winw, winh, editor_camera);
  }

  ImGui_StartFrame();

  for (int i = 0; i < 5; i++)
  {
    draw_panel(input, i);
  }

  ImGui_Render();
}

void EditorGui_HandleBrushInput(struct inputstate_t *input)
{
  static bool drawing = false;
  static ImVec2 start;

  if (gEditorGui_HoveredPanel < 2 || gEditorGui_HoveredPanel > 4)
    return;

  bool isTopPanel = (gEditorGui_HoveredPanel == 2);

  if (input->mbutton_left_toggle && !drawing && !edge_hovered && !edge_selected)
  {
    drawing = true;

    start = ImGui::GetMousePos();

    start.x -= panels[gEditorGui_HoveredPanel].pos.x;
    // start.y -= panels[gEditorGui_HoveredPanel].pos.y;

    start.y = (isTopPanel) ? panels[2].size.y - start.y : start.y - panels[gEditorGui_HoveredPanel].pos.y;
  }
  else if (input->mbutton_left_toggle && drawing)
  {
    drawing = false;

    ImVec2 end = ImGui::GetMousePos();

    end.x -= panels[gEditorGui_HoveredPanel].pos.x;
    // end.y -= panels[gEditorGui_HoveredPanel].pos.y;

    end.y = (isTopPanel) ? panels[2].size.y - end.y : end.y - panels[gEditorGui_HoveredPanel].pos.y;

    panel_finalise_brush(
        gEditorGui_HoveredPanel,
        panels[gEditorGui_HoveredPanel].size,
        start,
        end,
        panels[gEditorGui_HoveredPanel].cam_pos,
        panels[gEditorGui_HoveredPanel].cam_zoom,
        GRID_SPACING_WORLD);
  }

  // Edge selection
  if (input->mbutton_left_toggle && edge_hovered)
  {
    edge_selected = edge_hovered;

    edge_selected_a = edge_hovered_a;
    edge_selected_b = edge_hovered_b;
  }
  else if (input->mbutton_left_toggle && !drawing && !edge_hovered)
  {
    edge_selected = NULL;
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
  if (gEditorGui_HoveredPanel > 1 && input->mbutton_right)
  {
    panels[gEditorGui_HoveredPanel].cam_pos.x -= input->mx_rel / (*cam_zoom);
    panels[gEditorGui_HoveredPanel].cam_pos.y += input->my_rel / (*cam_zoom);
  }

  // Handle 3D panel relativity
  if (gEditorGui_HoveredPanel == 1 && input->mbutton_left_toggle)
  {
    gEditorGui_ViewportCaptured = true;
    SDL_SetWindowRelativeMouseMode(window, true);
  }
  if (gEditorGui_ViewportCaptured &&
      input->kCurrent[SDL_SCANCODE_ESCAPE] &&
      !input->kPrevious[SDL_SCANCODE_ESCAPE])
  {
    gEditorGui_ViewportCaptured = false;
    SDL_SetWindowRelativeMouseMode(window, false);
  }
  if (gEditorGui_ViewportCaptured)
  {
    Camera_Look(gCameras[gCameraIndex], input->mx_rel, input->my_rel);
  }
}
