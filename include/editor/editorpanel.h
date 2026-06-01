#ifndef EDITOR_PANEL_H
#define EDITOR_PANEL_H

#include "imgui/imgui.h"
#include "types/types_vector.h"

typedef enum panel_id_t {
  PANEL_VIEW_3D = 0,
  PANEL_ORTHO_TOP = 1,
  PANEL_ORTHO_SIDE = 2,
  PANEL_ORTHO_FRONT = 3,
  PANEL_UI_MAIN = 4,
  PANEL_UI_TOOLS = 5,
  PANEL_COUNT
} panel_id_t;

typedef enum panel_type_t {
  PANEL_TYPE_VIEW,
  PANEL_TYPE_ORTHO,
  PANEL_TYPE_UI
} panel_type_t;



typedef struct {
  ImVec2 pos, size;
  char label[16];

  panel_type_t type;

  ImVec2 cam_pos; // World coordinates
  float cam_zoom;

  Vector world_right;
  Vector world_up;

} gui_panel_t;

extern gui_panel_t panels[6];
extern int gEditorGui_HoveredPanel;

#endif



