#ifndef EDITOR_MAIN_H
#define EDITOR_MAIN_H


#include "editor/editorgui.h"
#include "editor/brush.h"









typedef struct{
  camera_t* camera;
  struct editor_input_t* input;
  editor_brush_array* b_arr;
  bool brush_resize; // Is currently resizing a brush?
} editor_state_t;

#ifdef __cplusplus
extern "C" {
#endif



void EditorInit(editor_state_t* eState, SDL_Window* window, SDL_GLContext glContext);

void EditorDestroy(editor_state_t* eState);


void EditorCreate_Brush(editor_brush_array* b_arr, Vector position, Vector scale);

void EditorLoop(
  SDL_Window* window,
  rdrawqueue_t* draw_q,
  camera_t* editor_camera,
  float mx, float my,
  int console_open
);

void EditorToggle(SDL_Window* window);

#ifdef __cplusplus
}
#endif

#endif
