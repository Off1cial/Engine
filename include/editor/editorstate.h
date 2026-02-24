#ifndef EDITOR_STATE_H
#define EDITOR_STATE_H

#include "editor/brush.h"
#include "rendering/camera.h"
#include "rendering/mesh.h"

typedef struct{
  brush_array_t* brush_array; 
  camera_t* camera;

  mesh_t* brush_meshes[4096];

} editor_state_t;


#endif
