#include "editor/editor.h"


void EditorInit(editor_state_t* eState, SDL_Window* window, SDL_GLContext glContext){
  
  EditorBrushArray_Init(eState->brush_array, 32);
  EditorGui_Init(window, glContext, eState->camera);

}

void EditorDestroy(editor_state_t* eState){
  EditorBrushArray_Destroy(eState->brush_array);
  free(eState->brush_array);
  free(eState);
}
