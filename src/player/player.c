#include "player/player.h"
#include "mem.h"
#include "application.h"

player_t *gPlayers = NULL;

void PlayerArrayInit(size_t capacity)
{
  gPlayers = malloc(sizeof(player_t));
  assert(gPlayers);

  size_t sfloats = sizeof(float) * capacity;
  gPlayers->x = ALIGNED_NEW(sfloats);
  gPlayers->y = ALIGNED_NEW(sfloats);
  gPlayers->z = ALIGNED_NEW(sfloats);

  gPlayers->vx = ALIGNED_NEW(sfloats);
  gPlayers->vy = ALIGNED_NEW(sfloats);
  gPlayers->vz = ALIGNED_NEW(sfloats);

  gPlayers->eyeheight = ALIGNED_NEW(sfloats);
  gPlayers->aabbheight = ALIGNED_NEW(sfloats);
  gPlayers->aabbwidth = ALIGNED_NEW(sfloats);

  gPlayers->aabbcx = ALIGNED_NEW(sfloats);
  gPlayers->aabbcy = ALIGNED_NEW(sfloats);
  gPlayers->aabbcz = ALIGNED_NEW(sfloats);
  
  gPlayers->grounded = ALIGNED_NEW(sizeof(bool) * capacity);
  gPlayers->camera = ALIGNED_NEW(sizeof(camera_t *) * capacity);
  gPlayers->count = 0;
  gPlayers->capacity = capacity;

  // Initial state
  for (int i = 0; i < capacity; i++)
  {
    gPlayers->camera[i] = NULL;
  }
  memset(gPlayers->x, 0, sfloats);
  memset(gPlayers->y, 0, sfloats);
  memset(gPlayers->z, 0, sfloats);

  memset(gPlayers->vx, 0, sfloats);
  memset(gPlayers->vy, 0, sfloats);
  memset(gPlayers->vz, 0, sfloats);

  memset(gPlayers->aabbheight, 0, sfloats);
  memset(gPlayers->aabbwidth, 0, sfloats);
  memset(gPlayers->aabbcx, 0, sfloats);
  memset(gPlayers->aabbcy, 0, sfloats);
  memset(gPlayers->aabbcz, 0, sfloats);

  memset(gPlayers->eyeheight, 0, sfloats);
  memset(gPlayers->grounded, 0, sizeof(sizeof(bool) * capacity));
}



int PlayerCreate(Vector position, camera_t *cam_override)
{
  if (gPlayers->count >= gPlayers->capacity)
  {
    printf("[PLAYERS]: Player limit reached, cannot allocate another player\n");
    return 0;
  }

  size_t count = gPlayers->count;
  float eyeh = 70.0f;

  gPlayers->x[count] = position.x;
  gPlayers->y[count] = position.y;
  gPlayers->z[count] = position.z;
  gPlayers->eyeheight[count] = eyeh;

  gPlayers->aabbheight[count] = 72.0f;
  gPlayers->aabbwidth[count] = 32.0f;

  // could technically save 16 bytes by removing cx and cz?
  gPlayers->aabbcx[count] = gPlayers->x[count];
  gPlayers->aabbcy[count] = gPlayers->y[count] + (0.5f * gPlayers->aabbheight[count]);
  gPlayers->aabbcz[count] = gPlayers->z[count];

  if (!cam_override)
  {
    camera_t *new_cam = ALIGNED_NEW(sizeof(camera_t));
    assert(new_cam);
    Camera_init(
        new_cam,
        (Vector){position.x, position.y + eyeh, position.z},
        (struct Viewport){.x = 0, .y = 0, .w = APPLIACTION_CONTAINER->win_w, APPLIACTION_CONTAINER->win_h});

    gPlayers->camera[count] = new_cam;
  }
  else
  {
    gPlayers->camera[count] = cam_override;
  }

  gPlayers->grounded[count] = false; // Let the BSP decide
  gPlayers->count++;

  return 1;
}
