#ifndef CAMERA_H
#define CAMERA_H

#include "types/types_vector.h"
#include <stdlib.h>

#define MAX_CAMERA_ROLL (M_PI * 2)

#define MAX_CAMERAS 5

#define WORLD_AXIS_X (Vector){1,0,0}
#define WORLD_AXIS_Y (Vector){0,1,0}
#define WORLD_AXIS_Z (Vector){0,0,1}


struct Viewport{
  float x,y,w,h;
};

typedef struct {
 Vector pos;
 Vector up,front,right;
 Vector worldUp;

 mat4 view;
 mat4 projection;

 float yaw,pitch,roll;
 float fov, aspect;

 float sens;

 bool fullscreen; // always uses entire viewport
 struct Viewport viewport;

 float far_plane, near_plane;



 bool drawWireframe;
} camera_t;

extern camera_t* gCameras[MAX_CAMERAS];
extern size_t gCameraCount;
extern size_t gCameraIndex;

typedef struct {
  Vector origin;
  Vector dir;
} Ray;

struct Camera2D{
  Vector2 pos;
  float zoom;
  mat4 projection, view;
  float width, height;
};

enum CAM_DIR{
  CAM_FRONT,
  CAM_RIGHT,
  CAM_LEFT,
  CAM_UP,
  CAM_DOWN,
  CAM_BACK
};

extern camera_t* activeCam;
extern Ray* activeCamCursorRay;


void Camera_ToggleWireframe(camera_t* cam);

void Camera_update(camera_t* cam);
void Camera_init(camera_t* cam, Vector position, struct Viewport view);

void Camera_Move(camera_t* cam, enum CAM_DIR direction, float unit);

bool RayIntersectsAABB(Vector origin, Vector dir, Vector aabbMin, Vector aabbMax, float *tOut);

bool RayIntersectsPlane(Vector origin, Vector dir, Vector pNormal, Vector pPos, float* t_out);

Ray Camera_ScreenPointToRay(camera_t* cam, float mouseX, float mouseY);


void Camera2D_Update(struct Camera2D* cam);
void Camera2D_Init(struct Camera2D* cam, Vector2 position, float width, float height);

void Camera2D_ScreenToWorld(struct Camera2D* cam, Vector2 in, Vector2 out);
void Camera2D_WorldToScreen(struct Camera2D* cam, Vector2 in, Vector2 out);

bool isCursorInViewport(float mx, float my, struct Viewport viewport);


bool isPosInBounds(float px, float py, float x, float y, float w, float h);
#endif
