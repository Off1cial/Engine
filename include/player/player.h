#ifndef PLAYER_H
#define PLAYER_H

#include "types/types_vector.h"
#include "rendering/camera.h"
#include <stddef.h>
#include <stdlib.h>

typedef struct player_t
{
  int *physbody_id; // index into gPhysbodyArray
  float *eyeheight;
  bool *grounded;
  camera_t **camera;
  size_t count, capacity;
} player_t;

extern player_t *gPlayers;

void PlayerArrayInit(size_t count);
int PlayerCreate(Vector position, camera_t *cam_override);

#endif