#ifndef EDITOR_CAMERA_H
#define EDITOR_CAMERA_H


#include "imgui/imgui.h"


ImVec2 EditorCamera_WorldToScreen(ImVec2 view_size, ImVec2 cam_pos, float cam_zoom, ImVec2 world_pos);
ImVec2 EditorCamera_ScreenToWorld(ImVec2 view_size, ImVec2 cam_pos, float cam_zoom, ImVec2 screen_pos);



#endif
