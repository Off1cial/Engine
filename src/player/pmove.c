#include "player/pmove.h"
#include "player/player.h"
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

void PlayerLook(int player, float xrel, float yrel){
  gPlayers->camera[player]->pos = (Vector){gPlayers->x[player], gPlayers->y[player] + gPlayers->eyeheight[player], gPlayers->z[player]};
  Camera_Look(gPlayers->camera[player], xrel, yrel);
}

static inline void player_check_grounded(int player){
  //printf("Player Y=%f\n", gPlayers->y[player]);
  gPlayers->grounded[player] = BSP_IsSolid(BSP_ACTIVE_TREE, (Vector){gPlayers->x[player], gPlayers->y[player] - 1, gPlayers->z[player]});
}


// Do not update position. Just update velocity
void PlayerMove(int player, float wishspeed, float accel, float friction, float gravity, float jump_velocity, float dt)
{
  if (player >= gPlayers->count)
  {
    return;
  }
  player_check_grounded(player);

  camera_t *cam = gPlayers->camera[player];

  Vector wish_dir = {0};
  float forward = 0;
  float right = 0;

  if (PRESSING_W(gInputState))
    forward += 1.0f;
  if (PRESSING_S(gInputState))
    forward -= 1.0f;
  if (PRESSING_D(gInputState))
    right += 1.0f;
  if (PRESSING_A(gInputState))
    right -= 1.0f;

  if (forward != 0 || right != 0)
  {
    Vector front = VectorScale(gPlayers->camera[player]->front, forward);
    Vector side = VectorScale(gPlayers->camera[player]->right, right);
    wish_dir = VectorNormalise(VectorAdd(front, side));
  }
  // Horizontal only
  Vector current_vel = (Vector){gPlayers->vx[player], 0.0f, gPlayers->vz[player]};
  Vector wish_vel = VectorScale(wish_dir, wishspeed);
  Vector wish_horiz = (Vector){wish_vel.x, 0.0f, wish_vel.z};

  Vector new_horiz = VectorMoveTowards(current_vel, wish_horiz, accel * dt);


  // apply friction


  // jumping and gravity
  if (gPlayers->grounded[player])
  {
    gPlayers->vy[player] = 0.0f;
    if (PRESSING_SPACE(gInputState))
    {
      gPlayers->vy[player] = jump_velocity;
    }
  }
  else
  {
    gPlayers->vy[player] -= gravity * dt;
  }


  gPlayers->vx[player] = new_horiz.x;
  gPlayers->vz[player] = new_horiz.z;

  //printf("Velocity: (%0.2f, %0.2f, %0.2f)\n", gPlayers->vx[player], gPlayers->vy[player], gPlayers->vz[player]);


  // TEMPORARY WHILE PHYSICS HASNT BEEN IMPLEMENTED 
  gPlayers->x[player] += gPlayers->vx[player] * dt;
  gPlayers->y[player] += gPlayers->vy[player] * dt;
  gPlayers->z[player] += gPlayers->vz[player] * dt;
}