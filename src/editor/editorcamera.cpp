#include "editor/editorcamera.h"


ImVec2 EditorCamera_WorldToScreen(ImVec2 view_size, ImVec2 cam_pos, float cam_zoom, ImVec2 world_pos){
  float x = (world_pos.x - cam_pos.x) * cam_zoom + view_size.x / 2.0f;
  float y = (cam_pos.y - world_pos.y) * cam_zoom + view_size.y / 2.0f;
  return ImVec2(x,y);
}


ImVec2 EditorCamera_ScreenToWorld(ImVec2 view_size, ImVec2 cam_pos, float cam_zoom, ImVec2 screen_pos){
  float x = (screen_pos.x - view_size.x / 2.0f) / cam_zoom + cam_pos.x; 
  float y = cam_pos.y - (screen_pos.y - view_size.y / 2.0f) / cam_zoom;
  return ImVec2(x,y );
}
