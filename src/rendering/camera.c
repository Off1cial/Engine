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

void Camera_update(camera_t *cam)
{
  // 1. Calculate front vector from yaw/pitch
  Vector front;
  front.x = cosf(RAD(cam->yaw)) * cosf(RAD(cam->pitch));
  front.y = sinf(RAD(cam->pitch));
  front.z = -sinf(RAD(cam->yaw)) * cosf(RAD(cam->pitch));
  VectorNormaliseTo(front, &cam->front);

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
  cam->far_plane = 1000.0f;

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

/*
void Camera_Move(struct camera_t* cam, enum CAM_DIR direction, float unit){
    vec3 mov;
    switch(direction){
        case CAM_FRONT:
            glm_vec3_scale(cam->front, unit, mov);
            glm_vec3_add(cam->pos, mov, cam->pos);
            break;
        case CAM_BACK:
            glm_vec3_scale(cam->front, -unit, mov);
            glm_vec3_add(cam->pos, mov, cam->pos);
            break;
        case CAM_RIGHT:
            glm_vec3_scale(cam->right, unit, mov);
            glm_vec3_add(cam->pos, mov, cam->pos);
            break;
        case CAM_LEFT:
            glm_vec3_scale(cam->right, -unit, mov);
            glm_vec3_add(cam->pos, mov, cam->pos);
            break;
        case CAM_UP:
            glm_vec3_scale(cam->worldUp, unit, mov);
            glm_vec3_add(cam->pos, mov, cam->pos);
            break;
        case CAM_DOWN:
            glm_vec3_scale(cam->worldUp, -unit, mov);
            glm_vec3_add(cam->pos, mov, cam->pos);
            break;
        default:
            break;
    }
}


void Camera2D_Update(struct Camera2D* cam){

  glm_ortho(0.0f, cam->width, cam->height, 0.0f, -1.0f, 1.0f, cam->projection);

  glm_mat4_identity(cam->view);
  glm_translate(
      cam->view,
      (vec3){cam->width/2.0f, cam->height/2.0f, 0.0f}
      );
  glm_scale(
      cam->view,
      (vec3){cam->zoom, cam->zoom, 1.0f}
      );
  glm_translate(cam->view,
      (vec3){-cam->pos[0], -cam->pos[1], 0.0f});

}

*/

// mouseX, mouseY: cursor position in window space
// winW, winH: window size
// view, proj: camera_t matrices
// camPos: world-space camera_t position

/*

ray_t Camera_ScreenPointToRay(camera_t *cam, float mouseX, float mouseY)
{
  ray_t ray;
  ray.dir = VECTOR_NAN;
  ray.origin = VECTOR_NAN;

  //printf("[RAYCAST]: MouseX: %0.2f  : MouseY: %0.2f\n", mouseX, mouseY);

  float vx = cam->viewport.x;
  float vy = cam->viewport.y;
  float vw = cam->viewport.w;
  float vh = cam->viewport.h;

  //printf("Ray cast in window : {%f, %f, %f, %f}\n", vx, vy, vw, vh);

  // Convert mouse to viewport-local coordinates
  float localX = mouseX - vx;
  float localY = mouseY - vy;



  //printf("[RAYCAST]: Input-{%0.3f, %0.3f}, Local-{%0.3f, %0.3f}\n", mouseX, mouseY, localX, localY);

  if (localX < 0 || localY < 0 || localX > vw || localY > vh)
  {
    return ray; // invalid ray
  }

  // NDC coordinates
  float x = (2.0f * localX) / vw - 1.0f;
  float y = 1.0f - (2.0f * localY) / vh;

  Vector4 ray_clip = {x, y, -1.0f, 1.0f};

  // Eye space
  mat4 invProj;
  Mat4Inverse(&cam->projection, &invProj);

  Vector4 ray_eye = Mat4Mulv(&invProj, &ray_clip);
  ray_eye.v[2] = -1.0f;
  ray_eye.v[3] = 0.0f;

  // World space
  mat4 invView;
  Mat4Inverse(&cam->view, &invView);

  Vector4 ray_world4 = Mat4Mulv(&invView, &ray_eye);

  ray.dir.v[0] = ray_world4.v[0];
  ray.dir.v[1] = ray_world4.v[1];
  ray.dir.v[2] = ray_world4.v[2];
  VectorNormalise(&ray.dir);

  ray.origin = cam->pos;

  printf("[RAYCAST]: Ray dot with up: %0.2f\n", VectorDot(ray.dir, cam->up));

  return ray;
}

*/

ray_t Camera_ScreenPointToRay(camera_t *cam, float mouseX, float mouseY)
{
  ray_t ray = {0};

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

  VectorNormalise(&rayDir);

  ray.origin = cam->pos;
  ray.dir = rayDir;

  return ray;
}

/*

bool RayIntersectsPlane(vec3 origin, vec3 dir, vec3 pNormal, vec3 pPos, float *t_out)
{
  vec3 U, V; // Plane vectors
             // vec3 helper = (fabs(N.y) < 0.9f) ? vec3(0,1,0) : vec3(1,0,0);
  vec3 helper;
  float dot_y_axis = glm_vec3_dot(pNormal, WORLD_AXIS_Y);
  if (dot_y_axis < 0.9f)
  {
    glm_vec3_copy(WORLD_AXIS_Y, helper);
  }
  else
  {
    glm_vec3_copy(WORLD_AXIS_X, helper);
  }

  glm_vec3_cross(helper, pNormal, U);
  glm_vec3_cross(pNormal, U, V);

  glm_vec3_normalize(U);
  glm_vec3_normalize(V);

  float denom = glm_vec3_dot(pNormal, dir);
  if (fabsf(denom) < 1e-6f)
    return false;

  vec3 sub;
  glm_vec3_sub(pPos, origin, sub);

  float t = glm_vec3_dot(sub, pNormal) / denom;
  if (t < 0.0f)
    return false;

  *t_out = t;

  return true;
}

bool RayIntersectsAABB(vec3 origin, vec3 dir, vec3 aabbMin, vec3 aabbMax, float *tOut)
{
  float tmin = -FLT_MAX;
  float tmax = FLT_MAX;

  for (int i = 0; i < 3; i++)
  {
    if (fabsf(dir[i]) < 1e-8f)
    {
      // Ray is parallel to this axis
      if (origin[i] < aabbMin[i] || origin[i] > aabbMax[i])
        return false; // Outside the slab → no hit
    }
    else
    {
      float invD = 1.0f / dir[i];
      float t1 = (aabbMin[i] - origin[i]) * invD;
      float t2 = (aabbMax[i] - origin[i]) * invD;

      if (t1 > t2)
      {
        float tmp = t1;
        t1 = t2;
        t2 = tmp;
      }

      if (t1 > tmin)
        tmin = t1;
      if (t2 < tmax)
        tmax = t2;

      if (tmin > tmax)
        return false; // Slabs don't overlap → no hit
    }
  }

  if (tmax < 0)
    return false; // Intersection behind the ray origin

  if (tOut)
    *tOut = tmin > 0 ? tmin : tmax;
  return true;
}

void Camera2D_Init(struct Camera2D *cam, vec2 position, float width, float height)
{
  if (cam == NULL)
    return;

  glm_vec2_copy(position, cam->pos);
  cam->width = width;
  cam->height = height;
  Camera2D_Update(cam);
}

void Camera2D_ScreenToWorld(struct Camera2D *cam, vec2 in, vec2 out)
{
  out[0] = (in[0] - cam->width / 2.0f) / cam->zoom + cam->pos[0];
  out[1] = (in[1] - cam->height / 2.0f) / cam->zoom + cam->pos[1];
}

void Camera2D_WorldToScreen(struct Camera2D *cam, vec2 in, vec2 out)
{
  out[0] = (in[0] - cam->pos[0]) * cam->zoom + cam->width / 2.0f;
  out[1] = (in[1] - cam->pos[1]) * cam->zoom + cam->height / 2.0f;
}

bool isCursorInViewport(float mx, float my, struct Viewport viewport)
{
  bool x = (viewport.x <= mx && mx <= viewport.x + viewport.w);
  bool y = (viewport.y <= my && my <= viewport.y + viewport.h);
  return (x && y);
}

bool isPosInBounds(float px, float py, float x, float y, float w, float h)
{
  return (x <= px && px <= x + w &&
          y <= py && py <= y + h);
}

*/

void Camera_Move(Vector direction, float scale, camera_t *cam)
{
  if (!cam)
  {
    return;
  }

  cam->pos = VectorAdd(VectorScale(direction, scale), cam->pos);
}