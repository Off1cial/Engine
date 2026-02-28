#ifndef EDITOR_GUI_H
#define EDITOR_GUI_H


#include "imgui_layer.h"
#include "rendering/camera.h"
#include "inputbase.h"

#ifdef __cplusplus
extern "C" {
#endif



void RecalculatePanels(int winw, int winh, camera_t* editor_camera);

void EditorGui_Init(SDL_Window* window, SDL_GLContext glContext, camera_t* editor_camera);
void EditorGui_DrawAll(SDL_Window* window, struct inputstate_t* input, camera_t* editor_camera, bool resize_flag);

void EditorGui_HandleBrushInput(struct inputstate_t* input);

void EditorGui_HandlePanelInput(SDL_Window* window, struct inputstate_t* input);


#ifdef __cplusplus
}
#endif


#endif
