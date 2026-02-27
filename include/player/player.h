#ifndef PLAYER_H
#define PLAYER_H

#include "types/types_vector.h"

#include "rendering/camera.h"

typedef struct {

  Vector position;
  camera_t* camera;

  size_t rigidbody;

  float forward, side;


} player_t;




#endif
