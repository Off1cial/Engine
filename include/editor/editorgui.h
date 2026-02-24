#ifndef EDITOR_GUI_H
#define EDITOR_GUI_H


#include "imgui_layer.h"
#include "rendering/camera.h"

#ifdef __cplusplus
extern "C" {
#endif

void EditorGui_Init(SDL_Window* window, SDL_GLContext glContext, camera_t* editor_camera);
void EditorGui_DrawAll(SDL_Window* window, camera_t* editor_camera, bool resize_flag);


#ifdef __cplusplus
}
#endif


#endif
