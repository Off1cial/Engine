#include "rendering/camera.h"
#include "types/types_vector.h"
#include "math/mathlib.h"
#include <math.h>
#include <stdio.h>

#include <glad/glad.h>

camera_t *gCameras[MAX_CAMERAS];
size_t gCameraCount = 0;
size_t gCameraIndex = 0;

void Camera_ToggleWireframe(camera_t *cam)
{
  cam->drawWireframe = !cam->drawWireframe;
}

void Camera_UpdateViewport(camera_t *cam, int x, int y, int w, int h, int winh, bool is_active)
{
  cam->viewport.x = x;
  cam->viewport.y = y;
  cam->viewport.w = w;
  cam->viewport.h = h;
  if (is_active)
  {
    glViewport(
        cam->viewport.x,
        winh - cam->viewport.y - cam->viewport.h,
        cam->viewport.w,
        cam->viewport.h);
  }
}

void Camera_update(camera_t *cam)
{
  cam->aspect = cam->viewport.w / cam->viewport.h;
  // 1. Calculate front vector from yaw/pitch
  Vector front;
  front.x = cosf(RAD(cam->yaw)) * cosf(RAD(cam->pitch));
  front.y = sinf(RAD(cam->pitch));
  front.z = -sinf(RAD(cam->yaw)) * cosf(RAD(cam->pitch));
  cam->front = VectorNormalise(front);

  Vector back = {-front.x, -front.y, -front.z};

  // 2. Recalculate right and up vectors
  cam->right = VectorCrossNormalise(cam->worldUp, back);
  cam->up = VectorCrossNormalise(back, cam->right);

  /*
  printf("CAMERA RIGHT: ");
  Vector_DPrint(&cam->right);
  printf("CAMERA FRONT: ");
  Vector_DPrint(&cam->front);
  printf("CAMERA UP: ");
  Vector_DPrint(&cam->up);
  */

  // 3. Create standard view matrix (yaw/pitch only)
  Vector centre = VectorAdd(cam->pos, cam->front);
  mat4 viewNoRoll = Mat4LookAt(cam->pos, centre, cam->up);

  // 4. Apply roll in camera_t space (around local Z)
  if (cam->roll != 0.0f)
  {
    mat4 rollMatrix = Mat4Rotate(RAD(cam->roll), (Vector){0, 0, -1});
    Mat4MulTo(&rollMatrix, &viewNoRoll, &cam->view);
  }
  else
  {
    Mat4Copy(viewNoRoll, &cam->view);
  }

  // 5. Projection
  cam->projection = Mat4Perspective(RAD(cam->fov), cam->aspect, cam->near_plane, cam->far_plane);
}

void Camera_init(camera_t *cam, Vector position, struct Viewport viewport)
{
  if (gCameraCount >= MAX_CAMERAS)
  {
    fprintf(stderr, "Camera limit reached\n");
    exit(1);
  }
  cam->pos = position;
  cam->worldUp = VectorInit(0, 1, 0);

  cam->yaw = 90.0f;
  cam->pitch = 0.0f;
  cam->roll = 0.0f;
  cam->viewport = viewport;
  cam->aspect = viewport.w / (float)viewport.h;
  cam->near_plane = 0.1f;
  cam->far_plane = 3000.0f;

  cam->fov = 60.0f;

  cam->drawWireframe = false;
  cam->fullscreen = true;

  Camera_update(cam);

  printf("camera_t created at: ");
  Vector_DPrint(&cam->pos);
  gCameras[gCameraCount++] = cam;
}

void Camera_Look(camera_t *cam, float xrel, float yrel)
{
  if (!cam)
  {
    return;
  }

  cam->yaw -= cam->sens * xrel;
  cam->pitch -= cam->sens * yrel;

  if (cam->pitch > 89.0f)
  {
    cam->pitch = 89.0f;
  }

  if (cam->pitch < -89.0f)
  {
    cam->pitch = -89.0f;
  }
  Camera_update(cam);
}


ray_t Camera_ScreenPointToRay(camera_t *cam, float mouseX, float mouseY)
{
  ray_t ray = {0};

  //printf("Raycast viewport (xywh): (%f, %f, %f, %f)\n", cam->viewport.x, cam->viewport.y, cam->viewport.w, cam->viewport.h);

  float vx = cam->viewport.x;
  float vy = cam->viewport.y;
  float vw = cam->viewport.w;
  float vh = cam->viewport.h;

  float localX = mouseX - vx;
  float localY = mouseY - vy;

  if (localX < 0 || localY < 0 ||
      localX > vw || localY > vh)
  {
    ray.dir = VECTOR_NAN;
    ray.origin = VECTOR_NAN;
    //printf("early exit\n");
    return ray;
  }

  // NDC
  float ndcX = (2.0f * localX / vw) - 1.0f;
  float ndcY = 1.0f - (2.0f * localY / vh);

  float aspect = vw / vh;

  float tanFov = tanf(RAD(cam->fov * 0.5f));

  Vector rayDir = cam->front;

  rayDir = VectorAdd(
      rayDir,
      VectorScale(cam->right, ndcX * aspect * tanFov));

  rayDir = VectorAdd(
      rayDir,
      VectorScale(cam->up, ndcY * tanFov));

  rayDir = VectorNormalise(rayDir);

  ray.origin = cam->pos;
  ray.dir = rayDir;

  //printf("Mouse Position: (%f, %f) -> Ray Dir: ", mouseX, mouseY); Vector_DPrint(&ray.dir);

  return ray;
}



void Camera_Move(Vector direction, float scale, camera_t *cam)
{
  if (!cam)
  {
    return;
  }

  cam->pos = VectorAdd(VectorScale(direction, scale), cam->pos);
}