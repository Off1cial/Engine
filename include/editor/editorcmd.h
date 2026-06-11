#ifndef EDITOR_CMD_H
#define EDITOR_CMD_H

#include "types/types_vector.h"
#include "editor/brush.h"
#include <stdlib.h>

// The inclusion of the editor queue prevents the brush data from changing during an ImGui loop

enum editor_cmd_type
{
  EDITOR_CMD_BRUSH_CREATE,
  EDITOR_CMD_BRUSH_DELETE,
  EDITOR_CMD_BRUSH_RESIZE,
  EDITOR_CMD_BRUSH_ROTATE
};

struct editor_cmd_t
{

  enum editor_cmd_type type;

  union
  {
    // Potential contents
    struct
    {
      int is_entity;

      vec_t startx;
      vec_t starty;
      vec_t startz;

      vec_t endx;
      vec_t endy;
      vec_t endz;

      vec_t sx;
      vec_t sy;
      vec_t sz;

      vec_t rx;
      vec_t ry;
      vec_t rz;

    } brush_create;

    size_t brush_delete;

    struct{
      size_t brush;
      float sx;
      float sy; 
      float sz;
    } brush_resize;

    struct {
      size_t brush;
      float rx;
      float ry;
      float rz;
    } brush_rotate;
  };

};

typedef struct {
  struct editor_cmd_t** data;
  size_t head, tail, capacity;

} editor_cmd_queue_t;

extern editor_cmd_queue_t* gEditorQueue;

#ifdef __cplusplus
extern "C" {
#endif

void EditorQueue_Init(editor_cmd_queue_t* q, size_t capacity);
void EditorQueue_Reset(editor_cmd_queue_t* q);
void EditorQueue_Push(editor_cmd_queue_t* q, struct editor_cmd_t* cmd);
void EditorQueue_Execute(editor_cmd_queue_t* q, editor_brush_array* arr);
void EditorQueue_Destroy(editor_cmd_queue_t* q);

void EditorCreate_BrushRoom(editor_brush_array *arr, Vector mins, Vector maxs, int material, int is_entity);

#ifdef __cplusplus
}
#endif

#endif
