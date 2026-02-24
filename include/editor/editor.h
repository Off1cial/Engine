#ifndef EDITOR_MAIN_H
#define EDITOR_MAIN_H

#include "editor/editorgui.h"
#include "editor/editorstate.h"
#include "editor/brush.h"


#ifdef __cplusplus
extern "C" {
#endif

void EditorInit(editor_state_t* eState, SDL_Window* window, SDL_GLContext glContext);

void EditorDestroy(editor_state_t* eState);

#ifdef __cplusplus
}
#endif

#endif
