#include "player/player.h"
#include "mem.h"
#include "application.h"
#include "physics/physbody.h"

player_t *gPlayers = NULL;

void PlayerArrayInit(size_t capacity)
{
  gPlayers = malloc(sizeof(player_t));
  assert(gPlayers);
  memset(gPlayers, 0, sizeof(player_t));   // zero the struct first

  gPlayers->capacity = capacity;
  gPlayers->count = 0;

  // Allocate all SoA arrays
  gPlayers->physbody_id = ALIGNED_NEW(sizeof(int) * capacity);
  gPlayers->eyeheight = ALIGNED_NEW(sizeof(float) * capacity);
  gPlayers->grounded = ALIGNED_NEW(sizeof(bool) * capacity);
  gPlayers->camera = ALIGNED_NEW(sizeof(camera_t*) * capacity);

  // Check for NULL (in case ALIGNED_NEW doesn't abort)
  if (!gPlayers->physbody_id || !gPlayers->eyeheight ||
    !gPlayers->grounded || !gPlayers->camera) {
    printf("Player array allocation failed\n");
    exit(1);
  }

  // Zero initialise all arrays
  memset(gPlayers->physbody_id, 0, sizeof(int) * capacity);
  memset(gPlayers->eyeheight, 0, sizeof(float) * capacity);
  memset(gPlayers->grounded, 0, sizeof(bool) * capacity);
  for (size_t i = 0; i < capacity; i++)
    gPlayers->camera[i] = NULL;
}

int PlayerCreate(Vector position, camera_t *cam_override)
{
  if (gPlayers->count >= gPlayers->capacity)
    return -1;

  int idx = gPlayers->count++;
  float eyeh = 35.0f;
  gPlayers->eyeheight[idx] = eyeh;
  gPlayers->grounded[idx] = false;

  // Create physics body (capsule or AABB)
  int physId = Physbody_AddAABB(position, VectorInit(16, 36, 16), 80.0f);
  // or Physbody_AddAABB(position, (Vector){16,36,16}, 80.0f);
  gPlayers->physbody_id[idx] = physId;

  // Camera
  if (!cam_override)
  {
    camera_t *nc = ALIGNED_NEW(sizeof(camera_t));
    Camera_init(nc, (Vector){position.x, position.y + eyeh, position.z},
                (struct Viewport){0, 0, APPLIACTION_CONTAINER->win_w, APPLIACTION_CONTAINER->win_h});
    gPlayers->camera[idx] = nc;
  }
  else
  {
    gPlayers->camera[idx] = cam_override;
  }
  return idx;
}
