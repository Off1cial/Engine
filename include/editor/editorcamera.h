#ifndef EDITOR_CAMERA_H
#define EDITOR_CAMERA_H


#include "imgui/imgui.h"

#ifdef __cplusplus
extern "C" {
#endif

ImVec2 EditorCamera_WorldToScreen(ImVec2 view_size, ImVec2 cam_pos, float cam_zoom, ImVec2 world_pos);

#ifdef __cplusplus
}
#endif


#endif
