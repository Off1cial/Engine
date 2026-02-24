#ifndef STATE_H
#define STATE_H

#include "application.h"
#include "mem.h"
#include "rendering/draw_list.h"
#include "rendering/camera.h"


typedef struct {
  struct container_t* app_container;
  rdrawqueue_t* draw_queue;
  struct mem_arena_t* arena;

  camera_t* cameras[MAX_CAMERAS];
  size_t camera_count, camera_active;
} state_t;

size_t State_AddCamera(state_t* state, camera_t* cam);


#endif
