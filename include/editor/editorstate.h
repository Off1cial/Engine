#ifndef EDITOR_STATE_H
#define EDITOR_STATE_H

#include "rendering/camera.h"
#include "editor/editorinput.h"

typedef struct{
  camera_t* camera;
  struct editor_input_t* input;
} editor_state_t;

void EditorState_Init();

#endif
