#ifndef EDITOR_PANEL_H
#define EDITOR_PANEL_H

#include "imgui/imgui.h"
#include "types/types_vector.h"


typedef struct {
  ImVec2 pos, size;
  char label[16];

  ImVec2 cam_pos; // World coordinates
  float cam_zoom;

  Vector world_right;
  Vector world_up;

} gui_panel_t;

extern gui_panel_t panels[5];
extern int gEditorGui_HoveredPanel;

#endif



