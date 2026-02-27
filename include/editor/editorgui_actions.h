#pragma once

#include "imgui/imgui.h"
#include "imgui_layer.h"
#include "editor/brush.h"
#include "inputbase.h"

void panel_finalise_brush(int panel, ImVec2 panel_size, ImVec2 start, ImVec2 end, ImVec2 cam_pos, float cam_zoom);
