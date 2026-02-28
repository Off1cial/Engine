#include "editor/editorgui_actions.h"
#include "editor/editorcmd.h"
#include "editor/editorcamera.h"
#include "mem.h"

void panel_finalise_brush(
  int panel,
  ImVec2 panel_size, 
  ImVec2 start, ImVec2 end, 
  ImVec2 cam_pos, float cam_zoom,
  int grid_spacing
){
  float axis_ignore_mask[3] = {1, 1, 1};


  ImVec2 w_start = EditorCamera_ScreenToWorld(
    panel_size,
    cam_pos,
    cam_zoom, 
    start
  );

  ImVec2 w_end = EditorCamera_ScreenToWorld(
    panel_size,
    cam_pos,
    cam_zoom,
    end
  );

  w_start.x = roundf(w_start.x / grid_spacing) * grid_spacing;
  w_start.y = roundf(w_start.y / grid_spacing) * grid_spacing;
  w_end.x   = roundf(w_end.x / grid_spacing) * grid_spacing;
  w_end.y   = roundf(w_end.y / grid_spacing) * grid_spacing;

  float centre[2] = {
    (w_end.x + w_start.x) * 0.5f,
    (w_end.y + w_start.y) * 0.5f
  };

  Vector brush_pos;
  float sx, sy, sz;

  switch(panel){
    case 2: {
      // TOP - Ignores Y 
      brush_pos.x = centre[0];
      brush_pos.y = 0;
      brush_pos.z = centre[1];

      sx = w_end.x - w_start.x;
      sy = 1.0f;
      sz = w_end.y - w_start.y;

      break;
    }
    case 3: {
      // SIDE - Ignores X
      brush_pos.x = 0;
      brush_pos.y = centre[1];
      brush_pos.z = centre[0];

      sx = 1.0f;
      sy = w_end.y - w_start.y;
      sz = w_end.x - w_start.x;

      break;
    }
    case 4: {
      // FRONT - Ignores Z
      brush_pos.x = centre[0];
      brush_pos.y = centre[1];
      brush_pos.z = 0;

      sx = w_end.x - w_start.x;
      sy = w_end.y - w_start.y;
      sz = 1.0f;

      break;
    }
    default: {
      return;
    }
  }


  printf("[Editor]: Attempting to allocate editor command:\n");
  printf("  Size: %zu\n", sizeof(struct editor_cmd_t));
  printf("  Alignment: %zu\n", alignof(struct editor_cmd_t));
  printf("  Current offset: %zu\n", gMemArena->offset);
  struct editor_cmd_t* cmd = (struct editor_cmd_t*)MEM_ARENA_ALLOC(
    gMemArena, 
    sizeof(struct editor_cmd_t), 
    alignof(struct editor_cmd_t)
  );
  
  cmd->type = EDITOR_CMD_BRUSH_CREATE;

  cmd->brush_create.px = brush_pos.x;
  cmd->brush_create.py = brush_pos.y;
  cmd->brush_create.pz = brush_pos.z;

  cmd->brush_create.sx = sx;
  cmd->brush_create.sy = sy;
  cmd->brush_create.sz = sz;


  cmd->brush_create.rx = 0;
  cmd->brush_create.ry = 0;
  cmd->brush_create.rz = 0;
   
  EditorQueue_Push(gEditorQueue, cmd);
}
