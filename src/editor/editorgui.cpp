#include "imgui_layer.h"
#include "editor/editorgui.h"
#include "editor/editorcamera.h"
#include "editor/editorgui_actions.h"
#include "editor/editorpanel.h"
#include "editor/editorcmd.h"
#include "editor/editorstate.h"
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

ImVec2 imvec2_add(ImVec2 a, ImVec2 b){
  return ImVec2(a.x + b.x, a.y + b.y);
}


gui_panel_t panels[5];

ImGuiWindowFlags panel_flags = 
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoBringToFrontOnFocus |
  ImGuiWindowFlags_NoCollapse;
  // Remove NoTitleBar


void RecalculatePanels(int winw, int winh, camera_t* editor_camera){
  
  const int p0_w = (int)(winw / 3);

  // Width of the 4 render panels:
  int rwidth = (winw - p0_w) / 2;
  int rheight = ( winh / 2 );
    
  ImVec2 rsize = ImVec2(rwidth, rheight);

  // Main editor panel
  panels[0] = {
    .pos = ImVec2(winw - p0_w, 0),
    .size = ImVec2(p0_w, winh),
    .label = "Main panel"
  };
  // Top left - 3D
  panels[1] = {
    .pos = ImVec2(0, 0),
    .size = rsize,
    .label = "3D"
  };
  // Top right  -TOP
  panels[2] = {
    .pos = ImVec2(rwidth, 0),
    .size = rsize,
    .label = "TOP"
  };
  // Bottom right - SIDE
  panels[3] = {
    .pos = ImVec2(rwidth, rheight),
    .size = rsize,
    .label = "SIDE"
  };
  // Bottom left - FRONT
  panels[4] = {
    .pos = ImVec2(0, rheight),
    .size = rsize,
    .label = "FRONT"
  };

  for (int i = 0; i < 5; i++){
    panels[i].cam_pos = ImVec2(0, 0);
    panels[i].cam_zoom = 1.0f;
  }

  editor_camera->viewport = (struct Viewport){0, 0, rsize.x, rsize.y};
  if (!start_up)glViewport(0, winh - rsize.y, rsize.x, rsize.y);
}

void EditorGui_Init(SDL_Window* window, SDL_GLContext glContext, camera_t* editor_camera){
  ImGui_Init(window, glContext);
  int winw, winh;
  SDL_GetWindowSize(window, &winw, &winh);
  printf("Editor Window: %dx%d\n", winw, winh);
  RecalculatePanels(winw, winh, editor_camera);
  start_up = false;
}

void draw_panel_grid(size_t index){

  if (index <= 1){ return; }
  float half_w = panels[index].size.x / (2.0f * panels[index].cam_zoom);
  float half_h = panels[index].size.y / (2.0f * panels[index].cam_zoom);

  ImVec2 cpos = panels[index].cam_pos;

  float left = cpos.x - half_w + (PANEL_SPACING_PX / 2.0f);
  float right = cpos.x + half_w - (PANEL_SPACING_PX / 2.0f);

  float top = cpos.y + half_h - (PANEL_SPACING_PX / 2.0f);
  float bottom = cpos.y - half_h + (PANEL_SPACING_PX / 2.0f);

  float start_x = floorf(left / GRID_SPACING_WORLD) * GRID_SPACING_WORLD;
  float start_y = floorf(bottom / GRID_SPACING_WORLD) * GRID_SPACING_WORLD;

  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  ImVec2 win_pos = ImGui::GetWindowPos();
  

  for (float x = start_x; x <= right; x += GRID_SPACING_WORLD){

    ImVec2 scr_a = EditorCamera_WorldToScreen(panels[index].size, panels[index].cam_pos, panels[index].cam_zoom, ImVec2(x, bottom));
    ImVec2 scr_b = EditorCamera_WorldToScreen(panels[index].size, panels[index].cam_pos, panels[index].cam_zoom, ImVec2(x, top));


    scr_a = imvec2_add(scr_a, win_pos);
    scr_b = imvec2_add(scr_b, win_pos);
   // Draw lines 
    draw_list->AddLine(scr_a, scr_b, IM_COL32(255,255,255,150), 0.8f);
  }

  for (float y = start_y; y <= top; y += GRID_SPACING_WORLD){
    
    ImVec2 scr_a =
      EditorCamera_WorldToScreen(panels[index].size, panels[index].cam_pos, panels[index].cam_zoom, ImVec2(left, y));
    ImVec2 scr_b =
      EditorCamera_WorldToScreen(panels[index].size, panels[index].cam_pos, panels[index].cam_zoom, ImVec2(right, y));

    scr_a = imvec2_add(scr_a, win_pos);
    scr_b = imvec2_add(scr_b, win_pos);
    draw_list->AddLine(scr_a, scr_b, IM_COL32(255,255,255,150),0.8f);
  }
  
}

void draw_brush_2d(int panel, size_t brush){
  if (panel <= 1) return;

  
  Vector bpos = VectorInit(
    gEditorBrushArray->px[brush],
    gEditorBrushArray->py[brush],
    gEditorBrushArray->pz[brush]
  );

  Vector scale_half = VectorInit(
    gEditorBrushArray->sx[brush] * 0.5f,
    gEditorBrushArray->sy[brush] * 0.5f,
    gEditorBrushArray->sz[brush] * 0.5f
  );
  
  ImVec2 wrld_a, wrld_b;

  Vector min = VectorSub(bpos, scale_half);
  Vector max = VectorAdd(bpos, scale_half);
  
  switch (panel){
    case 2 :{ // TOP
      wrld_a = ImVec2(min.x, min.z);
      wrld_b = ImVec2(max.x, max.z);
      break;
    }
    case 3 :{ // SIDE
      wrld_a = ImVec2(min.z, min.y);
      wrld_b = ImVec2(max.z, max.y);
      break;
    }
    case 4 :{ // FRONT
      wrld_a = ImVec2(min.x, min.y);
      wrld_b = ImVec2(max.x, max.y);
      break;
    }
  }

  ImVec2 scr_a = EditorCamera_WorldToScreen(
    panels[panel].size,
    panels[panel].cam_pos,
    panels[panel].cam_zoom,
    wrld_a
  );

  ImVec2 scr_b = EditorCamera_WorldToScreen(
    panels[panel].size,
    panels[panel].cam_pos,
    panels[panel].cam_zoom,
    wrld_b
  );
  

  /*
  printf("SCR_A: (%0.2f, %0.2f)\n", scr_a.x, scr_a.y);
  printf("SCR_B: (%0.2f, %0.2f)\n", scr_b.x, scr_b.y);
  */

  ImDrawList* draw_list =  ImGui::GetWindowDrawList();
  ImVec2 win_pos = ImGui::GetWindowPos();
  scr_a = imvec2_add(scr_a, win_pos);
  scr_b = imvec2_add(scr_b, win_pos);


  ImVec2 tl(fmin(scr_a.x, scr_b.x), fmin(scr_a.y, scr_b.y));
  ImVec2 br(fmax(scr_a.x, scr_b.x), fmax(scr_a.y, scr_b.y));
  draw_list->AddRect(tl, br, IM_COL32(200, 100, 0, 255));
}

void draw_main_panel_contents(){

}

void draw_panel(struct inputstate_t* input, size_t index){
  
  ImGui::SetNextWindowPos(panels[index].pos);
  ImGui::SetNextWindowSize(panels[index].size);
  
  ImGuiWindowFlags flags;

  if (index == 1){
    flags = panel_flags | ImGuiWindowFlags_NoBackground;
  }else{
    flags = panel_flags;
  }

  ImGui::Begin(panels[index].label, nullptr, flags);
  
  if (ImGui::IsWindowHovered()){
    gEditorGui_HoveredPanel = index;
  }

  draw_panel_grid(index);

  for (size_t b = 0; b < gEditorBrushArray->brush_count; b++){
    draw_brush_2d(index, b);
  } 



  ImGui::End();

}



void EditorGui_DrawAll(SDL_Window* window, struct inputstate_t* input, camera_t* editor_camera, bool resize_flag){
  
  if (resize_flag){
    int winw, winh;
    SDL_GetWindowSize(window, &winw, &winh);
    RecalculatePanels(winw, winh, editor_camera);
  }


  ImGui_StartFrame();
  
  for (int i = 0; i < 5; i++){
    draw_panel(input, i);
  }


  ImGui_Render();
}




void EditorGui_HandleBrushInput(struct inputstate_t* input)
{
    static bool drawing = false;
    static ImVec2 start;

    if (gEditorGui_HoveredPanel < 2 || gEditorGui_HoveredPanel > 4 )
        return;

    if (input->mbutton_left_toggle && !drawing)
    {
        drawing = true;
        start = ImGui::GetMousePos();
        start.x =  start.x - panels[gEditorGui_HoveredPanel].pos.x;
        start.y =  start.y - panels[gEditorGui_HoveredPanel].pos.y;


        printf("Start : (%0.2f, %0.2f)\n", start.x, start.y);
    
    }
    else if (input->mbutton_left_toggle && drawing)
    {
        drawing = false;

        ImVec2 end = ImGui::GetMousePos();

        end.x = end.x - panels[gEditorGui_HoveredPanel].pos.x; 
        end.y = end.y - panels[gEditorGui_HoveredPanel].pos.y; 
      

        panel_finalise_brush(
            gEditorGui_HoveredPanel,
            panels[gEditorGui_HoveredPanel].size,
            start,
            end,
            panels[gEditorGui_HoveredPanel].cam_pos,
            panels[gEditorGui_HoveredPanel].cam_zoom,
      GRID_SPACING_WORLD
        );
    }
}



void EditorGui_HandlePanelInput(struct inputstate_t* input){


  
  // Handle zoom
  panels[gEditorGui_HoveredPanel].cam_zoom += input->scrl_y * 0.1f;
  
  float* cam_zoom =  &panels[gEditorGui_HoveredPanel].cam_zoom;

  if (*cam_zoom < 0.5f) *cam_zoom = 0.5f;
  if (*cam_zoom > 5.0f) *cam_zoom = 5.0f;
}
