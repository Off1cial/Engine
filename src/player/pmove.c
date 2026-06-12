#include "player/pmove.h"
#include "player/player.h"
#include "physics/physbody.h"
#include "inputbase.h"
#include <editor/bsp.h>
#include <stdio.h>

#define PRESSING_W(input) ((input->kCurrent[SDL_SCANCODE_W]))
#define PRESSING_A(input) ((input->kCurrent[SDL_SCANCODE_A]))
#define PRESSING_S(input) ((input->kCurrent[SDL_SCANCODE_S]))
#define PRESSING_D(input) ((input->kCurrent[SDL_SCANCODE_D]))
#define PRESSING_SPACE(input) ((input->kCurrent[SDL_SCANCODE_SPACE]))

#define WISHSPEED 320.0f
#define ACCELERATION 2000.0f
#define FRICTION 6.0f
#define GRAVITY 800.0f
#define JUMP_VELOCITY 270.0f




static inline Vector VectorMoveTowards(Vector current, Vector target, float maxDelta)
{
  Vector diff = VectorSub(target, current);
  float dist = VectorMag(diff);
  if (dist <= maxDelta)
    return target;
  return VectorAdd(current, VectorScale(VectorNormalise(diff), maxDelta));
}

void PlayerLook(int player, float xrel, float yrel)
{
  Vector offset = { 0, gPlayers->eyeheight[player], 0 };
  gPlayers->camera[player]->pos = VectorAdd(PlayerPos(player), offset);
  Camera_Look(gPlayers->camera[player], xrel, yrel);
}



void PlayerMove(int player, float wishspeed, float accel, float friction,
                float gravity, float jumpVel, float dt)
{
  int id = gPlayers->physbody_id[player];
  physbody_array_t *b = gPhysbodyArray;
  camera_t *cam = gPlayers->camera[player];
  bool grounded = gPlayers->grounded[player];

  // Wish direction
  Vector wishDir = {0};
  float forward = 0, right = 0;
  if (PRESSING_W(gInputState))
    forward += 1;
  if (PRESSING_S(gInputState))
    forward -= 1;
  if (PRESSING_D(gInputState))
    right += 1;
  if (PRESSING_A(gInputState))
    right -= 1;
  if (forward || right)
  {
    // Flatten camera vectors so looking up/down doesn't affect speed
    Vector flatFront = { cam->front.x, 0.0f, cam->front.z };
    Vector flatRight = { cam->right.x, 0.0f, cam->right.z };

    // Avoid division by zero if the camera looks straight up/down
    if (VectorMag(flatFront) < 0.0001f) flatFront = (Vector){ 0, 0, 1 };
    if (VectorMag(flatRight) < 0.0001f) flatRight = (Vector){ 1, 0, 0 };

    flatFront = VectorNormalise(flatFront);
    flatRight = VectorNormalise(flatRight);

    Vector f = VectorScale(flatFront, forward);
    Vector r = VectorScale(flatRight, right);
    wishDir = VectorNormalise(VectorAdd(f, r));
  }

  // Horizontal acceleration
  Vector curVel = {b->vx[id], 0, b->vz[id]};
  Vector wishVel = VectorScale(wishDir, wishspeed);
  Vector wishHoriz = {wishVel.x, 0, wishVel.z};
  Vector newHoriz = VectorMoveTowards(curVel, wishHoriz, accel * dt);

  // Friction when no input & on ground
  if (grounded && forward == 0 && right == 0)
  {
    float spd = VectorMag(newHoriz);
    if (spd > 0.1f)
    {
      float drop = spd * friction * dt;
      float ns = fmaxf(0, spd - drop);
      newHoriz = VectorScale(VectorNormalise(newHoriz), ns);
    }
    else
      newHoriz = (Vector){0};
  }

  b->vx[id] = newHoriz.x;
  b->vz[id] = newHoriz.z;

  // Vertical / jump
  if (grounded)
  {
    b->vy[id] = 0;
    if (PRESSING_SPACE(gInputState)) {
      //printf("JUMP! vy set to %.2f\n", jumpVel);
      b->vy[id] = jumpVel;
      gPlayers->grounded[player] = false;
    }
  }
  // gravity is applied in PhysbodyArray_Step, so we don't subtract here.
}