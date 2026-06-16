#include "physics/physbody.h"
#include "mem.h"

physbody_array_t *gPhysbodyArray = NULL;

void PhysbodyArray_Init(size_t capacity)
{
  gPhysbodyArray = malloc(sizeof(physbody_array_t));
  assert(gPhysbodyArray);

  gPhysbodyArray->count = 0;
  gPhysbodyArray->capacity = capacity;

  size_t sfloats = sizeof(float) * capacity;

  gPhysbodyArray->x = ALIGNED_NEW(sfloats);
  gPhysbodyArray->y = ALIGNED_NEW(sfloats);
  gPhysbodyArray->z = ALIGNED_NEW(sfloats);
  memset(gPhysbodyArray->x, 0, sfloats);
  memset(gPhysbodyArray->y, 0, sfloats);
  memset(gPhysbodyArray->z, 0, sfloats);
  gPhysbodyArray->vx = ALIGNED_NEW(sfloats);
  gPhysbodyArray->vy = ALIGNED_NEW(sfloats);
  gPhysbodyArray->vz = ALIGNED_NEW(sfloats);
  memset(gPhysbodyArray->vx, 0, sfloats);
  memset(gPhysbodyArray->vy, 0, sfloats);
  memset(gPhysbodyArray->vz, 0, sfloats);
  gPhysbodyArray->ax = ALIGNED_NEW(sfloats);
  gPhysbodyArray->ay = ALIGNED_NEW(sfloats);
  gPhysbodyArray->az = ALIGNED_NEW(sfloats);
  memset(gPhysbodyArray->ax, 0, sfloats);
  memset(gPhysbodyArray->ay, 0, sfloats);
  memset(gPhysbodyArray->az, 0, sfloats);
  gPhysbodyArray->fx = ALIGNED_NEW(sfloats);
  gPhysbodyArray->fy = ALIGNED_NEW(sfloats);
  gPhysbodyArray->fz = ALIGNED_NEW(sfloats);
  memset(gPhysbodyArray->fx, 0, sfloats);
  memset(gPhysbodyArray->fy, 0, sfloats);
  memset(gPhysbodyArray->fz, 0, sfloats);

  gPhysbodyArray->inv_mass = ALIGNED_NEW(sfloats);
  memset(gPhysbodyArray->inv_mass, 0, sfloats);

  // Collider data
  gPhysbodyArray->ctype = malloc(sizeof(collider_type_t) * capacity);

  gPhysbodyArray->radius = ALIGNED_NEW(sfloats);
  gPhysbodyArray->height = ALIGNED_NEW(sfloats);
  gPhysbodyArray->halfx = ALIGNED_NEW(sfloats);
  gPhysbodyArray->halfy = ALIGNED_NEW(sfloats);
  gPhysbodyArray->halfz = ALIGNED_NEW(sfloats);
  memset(gPhysbodyArray->radius, 0, sfloats);
  memset(gPhysbodyArray->height, 0, sfloats);
  memset(gPhysbodyArray->halfx, 0, sfloats);
  memset(gPhysbodyArray->halfy, 0, sfloats);
  memset(gPhysbodyArray->halfz, 0, sfloats);

  gPhysbodyArray->mesh_index = malloc(sizeof(int) * capacity);
  memset(gPhysbodyArray->mesh_index, 0, sizeof(int) * capacity);
  gPhysbodyArray->grounded = malloc(sizeof(int) * capacity);
  memset(gPhysbodyArray->grounded, 0, sizeof(int) * capacity);
}

static int Physbody_AddInternal(Vector pos, float mass)
{
  if (gPhysbodyArray->count >= gPhysbodyArray->capacity)
    return -1;

  int idx = gPhysbodyArray->count++;

  gPhysbodyArray->x[idx] = pos.x;
  gPhysbodyArray->y[idx] = pos.y;
  gPhysbodyArray->z[idx] = pos.z;
  gPhysbodyArray->inv_mass[idx] = (mass > 0.0f) ? (1.0f / mass) : 0.0f;

  return idx;
}

int Physbody_AddAABB(Vector pos, Vector half, float mass)
{
  int idx = Physbody_AddInternal(pos, mass);
  if (idx < 0)
    return -1;

  gPhysbodyArray->ctype[idx] = PHYSCOLLIDER_AABB;
  gPhysbodyArray->halfx[idx] = half.x;
  gPhysbodyArray->halfy[idx] = half.y;
  gPhysbodyArray->halfz[idx] = half.z;

  return idx;
}

int Physbody_AddSphere(Vector pos, float radius, float mass)
{
  int idx = Physbody_AddInternal(pos, mass);
  if (idx < 0)
    return -1;

  gPhysbodyArray->ctype[idx] = PHYSCOLLIDER_SPHERE;
  gPhysbodyArray->radius[idx] = radius;

  return idx;
}

int Physbody_AddCapsule(Vector pos, float radius, float height, float mass)
{
  int idx = Physbody_AddInternal(pos, mass);
  if (idx < 0)
    return -1;

  gPhysbodyArray->ctype[idx] = PHYSCOLLIDER_CAPSULE;
  gPhysbodyArray->radius[idx] = radius;
  gPhysbodyArray->height[idx] = height;

  return idx;
}

void PhysbodyArray_Step(float dt, Vector gravity)
{
  for (size_t i = 0; i < gPhysbodyArray->count; i++)
  {
    float invM = gPhysbodyArray->inv_mass[i];
    if (invM <= 0.0f)
      continue; // static body

    // ---- 1. Compute acceleration from forces ----
    float ax = gPhysbodyArray->fx[i] * invM;
    float ay = gPhysbodyArray->fy[i] * invM;
    float az = gPhysbodyArray->fz[i] * invM;

    // Add gravity (world‑space, here Y is up)
    if (!gPhysbodyArray->grounded[i]) 
    ay += gravity.y;
    

    // Store acceleration if you need it elsewhere (optional)
    gPhysbodyArray->ax[i] = ax;
    gPhysbodyArray->ay[i] = ay;
    gPhysbodyArray->az[i] = az;

    // ---- 2. Integrate velocity ----
    gPhysbodyArray->vx[i] += ax * dt;
    gPhysbodyArray->vy[i] += ay * dt;
    gPhysbodyArray->vz[i] += az * dt;

    // ---- 3. Integrate position ----
    gPhysbodyArray->x[i] += gPhysbodyArray->vx[i] * dt;
    gPhysbodyArray->y[i] += gPhysbodyArray->vy[i] * dt;
    gPhysbodyArray->z[i] += gPhysbodyArray->vz[i] * dt;
  }

  // ---- 4. Clear forces for next frame ----
  size_t byteSize = sizeof(float) * gPhysbodyArray->capacity;
  memset(gPhysbodyArray->fx, 0, byteSize);
  memset(gPhysbodyArray->fy, 0, byteSize);
  memset(gPhysbodyArray->fz, 0, byteSize);
}
