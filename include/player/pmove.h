#ifndef PMOVE_H
#define PMOVE_H

#include "physics/physbody.h"
#include "player/player.h"
#include "editor/bsp.h"

void PlayerLook(int player, float xrel, float yrel);
void PlayerMove(int player, float wishspeed, float accel, float friction, float gravity, float jump_velocity, float dt);

static inline Vector PlayerPos(int player) {
    int id = gPlayers->physbody_id[player];
    return (Vector){gPhysbodyArray->x[id], gPhysbodyArray->y[id], gPhysbodyArray->z[id]};
}
static inline Vector PlayerVel(int player) {
    int id = gPlayers->physbody_id[player];
    return (Vector){gPhysbodyArray->vx[id], gPhysbodyArray->vy[id], gPhysbodyArray->vz[id]};
}
static inline int PlayerGrounded(int player) { return gPlayers->grounded[player]; }

static inline void PlayerUpdateGrounded(int player)
{
  int id = gPlayers->physbody_id[player];
  float halfH = gPhysbodyArray->halfy[id];          // capsule half‑height (e.g. 36)
  Vector feet = PlayerPos(player);
  feet.y -= halfH;                                   // bottom of the capsule
  gPlayers->grounded[player] = BSP_IsSolid(BSP_ACTIVE_TREE,
    (Vector) {
    feet.x, feet.y - 0.1f, feet.z
  });     // just below the feet
  gPhysbodyArray->grounded[id] = gPlayers->grounded[player];
}


static inline void PlayerGroundClamp(int player)
{
  int id = gPlayers->physbody_id[player];
  float halfH = gPhysbodyArray->halfy[id];
  Vector pos = PlayerPos(player);

  // Only clamp if the player is actually inside solid (penetrating the floor)
  Vector bottom = { pos.x, pos.y - halfH, pos.z };
  if (!BSP_IsSolid(BSP_ACTIVE_TREE, bottom))
    return;                     // not penetrating, leave alone

  // Scan upward until we find a non‑solid point exactly at the surface
  float testY = pos.y - halfH;
  while (testY < pos.y + halfH + 1.0f)
  {
    if (!BSP_IsSolid(BSP_ACTIVE_TREE, (Vector) { pos.x, testY, pos.z }))
    {
      // Snap feet to this Y, center = testY + halfH
      gPhysbodyArray->y[id] = testY + halfH;
      gPhysbodyArray->vy[id] = 0;
      gPlayers->grounded[player] = true;
      return;
    }
    testY += 0.5f;
  }
}


static bool PlayerAABBIntersectsSolid(Vector center, float hw, float hh, float hd)
{
  // Same 8‑corner test as before
  Vector corners[8] = {
      {center.x - hw, center.y - hh, center.z - hd},
      {center.x + hw, center.y - hh, center.z - hd},
      {center.x - hw, center.y + hh, center.z - hd},
      {center.x + hw, center.y + hh, center.z - hd},
      {center.x - hw, center.y - hh, center.z + hd},
      {center.x + hw, center.y - hh, center.z + hd},
      {center.x - hw, center.y + hh, center.z + hd},
      {center.x + hw, center.y + hh, center.z + hd}
  };
  for (int i = 0; i < 8; i++)
    if (BSP_IsSolid(BSP_ACTIVE_TREE, corners[i]))
      return true;
  return false;
}

static void PlayerCollideWorld(int player)
{
  int id = gPlayers->physbody_id[player];
  float hw = gPhysbodyArray->halfx[id];
  float hh = gPhysbodyArray->halfy[id];
  float hd = gPhysbodyArray->halfz[id];
  Vector pos = PlayerPos(player);
  Vector vel = PlayerVel(player);

  // If not intersecting, nothing to do
  if (!PlayerAABBIntersectsSolid(pos, hw, hh, hd))
    return;
  // ---- Penetration resolution ----
  // Try moving back along each axis independently
  Vector origPos = pos;
  const float step = 0.5f;

  // X axis
  if (vel.x != 0) {
    Vector test = pos;
    while (PlayerAABBIntersectsSolid(test, hw, hh, hd) && fabsf(test.x - origPos.x) < hw * 2) {
      test.x -= (vel.x > 0 ? step : -step);
    }
    if (!PlayerAABBIntersectsSolid(test, hw, hh, hd)) {
      gPhysbodyArray->x[id] = test.x;
      gPhysbodyArray->vx[id] = 0;
      return;
    }
  }

  // Z axis
  if (vel.z != 0) {
    Vector test = pos;
    while (PlayerAABBIntersectsSolid(test, hw, hh, hd) && fabsf(test.z - origPos.z) < hd * 2) {
      test.z -= (vel.z > 0 ? step : -step);
    }
    if (!PlayerAABBIntersectsSolid(test, hw, hh, hd)) {
      gPhysbodyArray->z[id] = test.z;
      gPhysbodyArray->vz[id] = 0;
      return;
    }
  }

  // If completely stuck (shouldn't happen), teleport to last known good position.
  // For now, zero all velocity.
  gPhysbodyArray->vx[id] = 0;
  gPhysbodyArray->vz[id] = 0;
}

#endif
