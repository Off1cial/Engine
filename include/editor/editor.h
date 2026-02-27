#ifndef EDITOR_MAIN_H
#define EDITOR_MAIN_H

#include "editor/editorgui.h"
#include "editor/editorstate.h"
#include "editor/brush.h"


#ifdef __cplusplus
extern "C" {
#endif

extern bool* gEditorActive;

void EditorInit(editor_state_t* eState, SDL_Window* window, SDL_GLContext glContext);

void EditorDestroy(editor_state_t* eState);

void EditorLoop(
  SDL_Window* window,
  rdrawqueue_t* draw_q,
  camera_t* editor_camera
);

void EditorToggle(SDL_Window* window);

#ifdef __cplusplus
}
#endif

#endif
