#ifndef PLAYER_H
#define PLAYER_H

#include "types/types_vector.h"
#include "rendering/camera.h"
#include <stddef.h>
#include <stdlib.h>

typedef struct player_t {
  // Positions
  float* x;
  float* y;
  float* z;
  // Velocity
  float* vx;
  float* vy;
  float* vz;

  // AABB Dimensions
  float* aabbheight;
  float* aabbwidth;
  // AABB centre - avoid recalculating when drawing
  float* aabbcx;
  float* aabbcy;
  float* aabbcz;
  

  //float* yaw;
  //float* pitch;
  camera_t** camera;

  float* eyeheight;

  bool* grounded;

  size_t count;
  size_t capacity;
} player_t;

extern player_t* gPlayers;

void PlayerArrayInit(size_t count);
int PlayerCreate(Vector position, camera_t *cam_override);

#endif