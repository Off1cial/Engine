#ifndef EDITOR_STATE_H
#define EDITOR_STATE_H

#include "editor/brush.h"
#include "rendering/camera.h"

typedef struct{
  brush_array_t* brush_array; 
  camera_t* camera;

} editor_state_t;


#endif
