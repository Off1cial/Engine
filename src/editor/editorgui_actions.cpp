#include "editor/editorgui_actions.h"
#include "editor/editorcmd.h"
#include "editor/editorcamera.h"
#include "mem.h"
#include "editor/editorpanel.h"

#include "types/types_vector.h"




void panel_finalise_brush(
  int panel,
  ImVec2 panel_size,
  ImVec2 start,
  ImVec2 end,
  ImVec2 cam_pos,
  float cam_zoom,
  int grid_spacing)
{

  
  ImVec2 w_start = EditorCamera_ScreenToWorld(
      panel_size,
      cam_pos,
      cam_zoom,
      start);

  ImVec2 w_end = EditorCamera_ScreenToWorld(
      panel_size,
      cam_pos,
      cam_zoom,
      end);

  w_start.x = roundf(w_start.x / grid_spacing) * grid_spacing;
  w_start.y = roundf(w_start.y / grid_spacing) * grid_spacing;

  w_end.x = roundf(w_end.x / grid_spacing) * grid_spacing;
  w_end.y = roundf(w_end.y / grid_spacing) * grid_spacing;

  float minx = fminf(w_start.x, w_end.x);
  float maxx = fmaxf(w_start.x, w_end.x);

  float miny = fminf(w_start.y, w_end.y);
  float maxy = fmaxf(w_start.y, w_end.y);

  float sx, sy, sz;

  Vector world_start = VECTOR_ZERO;
  Vector world_end = VECTOR_ZERO;

  printf("START SCREEN: %f %f\n", start.x, start.y);
  printf("END SCREEN: %f %f\n", end.x, end.y);

  printf("WORLD START: %f %f\n", w_start.x, w_start.y);
  printf("WORLD END: %f %f\n", w_end.x, w_end.y);

  switch(panel)
  {
    case  PANEL_ORTHO_TOP: // TOP
      sx = maxx - minx;
      sy = BRUSH_DEFUALT_SCALE;
      sz = maxy - miny;

      world_start = VectorInit(minx, 0, miny);
      world_end   = VectorInit(maxx, BRUSH_DEFUALT_SCALE, maxy);
      break;

    case PANEL_ORTHO_SIDE: // SIDE
      sx = BRUSH_DEFUALT_SCALE;
      sy = maxy - miny;
      sz = maxx - minx;

      world_start = VectorInit(0, miny, minx);
      world_end   = VectorInit(BRUSH_DEFUALT_SCALE, maxy, maxx);
      break;

    case PANEL_ORTHO_FRONT: // FRONT
      sx = maxx - minx;
      sy = maxy - miny;
      sz = BRUSH_DEFUALT_SCALE;

      world_start = VectorInit(minx, miny, 0);
      world_end   = VectorInit(maxx, maxy, BRUSH_DEFUALT_SCALE);
      break;

    default:
      return;
  }

  struct editor_cmd_t* cmd = (struct editor_cmd_t*)
    MEM_ARENA_ALLOC(
      gMemArena,
      sizeof(struct editor_cmd_t),
      alignof(struct editor_cmd_t));

  cmd->type = EDITOR_CMD_BRUSH_CREATE;

  cmd->brush_create.startx = world_start.x;
  cmd->brush_create.starty = world_start.y;
  cmd->brush_create.startz = world_start.z;

  cmd->brush_create.endx = world_end.x;
  cmd->brush_create.endy = world_end.y;
  cmd->brush_create.endz = world_end.z;

  cmd->brush_create.sx = sx;
  cmd->brush_create.sy = sy;
  cmd->brush_create.sz = sz;

  cmd->brush_create.rx = 0;
  cmd->brush_create.ry = 0;
  cmd->brush_create.rz = 0;

  cmd->brush_create.is_entity = 0;

  EditorQueue_Push(gEditorQueue, cmd);
}


