#include "imgui_layer.h"
#include "editor/editorgui.h"
#include "editor/editorcamera.h"
#include "editor/editorstate.h"
#include "imgui/imgui.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <cmath>

#define MAX_LABEL_LENGTH 32
#define GRID_SPACING_WORLD 10
#define PANEL_SPACING_PX (int)2

ImVec2 imvec2_add(ImVec2 a, ImVec2 b){
  return ImVec2(a.x + b.x, a.y + b.y);
}


typedef struct {
  ImVec2 pos, size;
  char label[16];

  ImVec2 cam_pos; // World coordinates
  float cam_zoom;
} gui_panel_t;

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
  panels[1] = {
    .pos = ImVec2(0, 0),
    .size = rsize,
    .label = "3D"
  };
  panels[2] = {
    .pos = ImVec2(rwidth, 0),
    .size = rsize,
    .label = "TOP"
  };
  panels[3] = {
    .pos = ImVec2(rwidth, rheight),
    .size = rsize,
    .label = "SIDE"
  };
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
}

void EditorGui_Init(SDL_Window* window, SDL_GLContext glContext, camera_t* editor_camera){
  ImGui_Init(window, glContext);
  int winw, winh;
  SDL_GetWindowSize(window, &winw, &winh);
  printf("Editor Window: %dx%d\n", winw, winh);
  RecalculatePanels(winw, winh, editor_camera);
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
    draw_list->AddLine(scr_a, scr_b, IM_COL32(255,255,255,255), 1.0f);
  }

  for (float y = start_y; y <= top; y += GRID_SPACING_WORLD){
    
    ImVec2 scr_a =
      EditorCamera_WorldToScreen(panels[index].size, panels[index].cam_pos, panels[index].cam_zoom, ImVec2(left, y));
    ImVec2 scr_b =
      EditorCamera_WorldToScreen(panels[index].size, panels[index].cam_pos, panels[index].cam_zoom, ImVec2(right, y));

    scr_a = imvec2_add(scr_a, win_pos);
    scr_b = imvec2_add(scr_b, win_pos);
    draw_list->AddLine(scr_a, scr_b, IM_COL32(255,255,255,255), 1.0f);
  }
  
}

void draw_panel(size_t index){
  
  ImGui::SetNextWindowPos(panels[index].pos);
  ImGui::SetNextWindowSize(panels[index].size);
  


  ImGui::Begin(panels[index].label, nullptr, panel_flags);

  draw_panel_grid(index);

  ImGui::End();

}



void EditorGui_DrawAll(SDL_Window* window, camera_t* editor_camera, bool resize_flag){
  
  if (resize_flag){
    int winw, winh;
    SDL_GetWindowSize(window, &winw, &winh);
    RecalculatePanels(winw, winh, editor_camera);
  }


  ImGui_StartFrame();
  
  for (int i = 0; i < 5; i++){
    draw_panel(i);
  }

  ImGui_Render();
}


