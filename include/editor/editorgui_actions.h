#pragma once

#include "imgui/imgui.h"
#include "imgui_layer.h"
#include "editor/brush.h"
#include "editor/editorcmd.h"
#include "inputbase.h"

#define EDITOR_HISTORY_SIZE 32

void panel_finalise_brush(int panel, ImVec2 panel_size, ImVec2 start, ImVec2 end, ImVec2 cam_pos, float cam_zoom, int grid_spacing);



typedef struct editor_action_history_t{
  
  editor_cmd_t cmds[EDITOR_HISTORY_SIZE];
  int tail, head;

} editor_action_history;