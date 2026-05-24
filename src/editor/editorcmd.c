#include "editor/editorcmd.h"
#include <stdio.h>

int grow_brush_arr(editor_brush_array *b_arr)
{
  if (b_arr->capacity * 2 < MAX_BRUSHES)
  {
    b_arr->capacity *= 2;
  }
  else
  {
    b_arr->capacity = MAX_BRUSHES;
  }
  b_arr->brushes = realloc(b_arr->brushes, sizeof(brush_t) * b_arr->capacity);

  if (!b_arr->brushes)
  {
    fprintf(stderr, "[EDITOR]: Failed to grow brush array\n");
    return 0;
  }
  return 1;
}

void EditorCreate_Brush(editor_brush_array *b_arr, Vector start, Vector end, Vector scale, int is_entity)
{
  if (b_arr->count >= b_arr->capacity)
  {
    if (!grow_brush_arr(b_arr))
    {
      exit(1);
    }
  }

  Vector mins = {
      fminf(start.x, end.x),
      fminf(start.y, end.y),
      fminf(start.z, end.z)};
  Vector maxs = {
      fmaxf(start.x, end.x),
      fmaxf(start.y, end.y),
      fmaxf(start.z, end.z)};

  brush_t new_brush = make_brush_cube(mins, maxs, 0);
  new_brush.is_entity = is_entity;
  b_arr->brushes[b_arr->count] = new_brush;
  BrushToMesh(&gEditorBrushArray->brushes[b_arr->count], &gEditorBrushArray->brushes[b_arr->count].editor_mesh);
  MeshRecalculateNormals(&gEditorBrushArray->brushes[b_arr->count].editor_mesh);
  MeshUpload(&gEditorBrushArray->brushes[b_arr->count].editor_mesh, GL_STATIC_DRAW);
  gEditorBrushArray->brushes[b_arr->count].dirty = 0;

  b_arr->count++;

  printf("[EDITOR]: Brush created, S{%0.3f, %0.3f, %0.3f}, E{%0.3f, %0.3f, %0.3f}\n", mins.x, mins.y, mins.z, maxs.x, maxs.y, maxs.z);
}

void EditorQueue_Init(editor_cmd_queue_t *q, size_t capacity)
{
  q->data = malloc(sizeof(struct editor_cmd_t) * capacity);

  q->head = 0;
  q->tail = 0;
  q->capacity = capacity;
}

void EditorQueue_Destroy(editor_cmd_queue_t *q)
{
  if (!q)
    return;

  free(q->data);
  q->data = NULL;
  q->head = q->tail = q->capacity = 0;
}

void EditorQueue_Reset(editor_cmd_queue_t *q)
{
  q->head = 0;
  q->tail = 0;
}

void EditorQueue_Push(editor_cmd_queue_t *q, struct editor_cmd_t *cmd)
{
  size_t next = (q->tail + 1) % q->capacity;
  if (next == q->head)
  {
    fprintf(stderr, "[EditorQueue]: Overflow!\n");
    exit(1);
  }
  q->data[q->tail] = cmd;
  q->tail = next;
}

void EditorQueue_Execute(editor_cmd_queue_t *q, editor_brush_array *arr)
{
  while (q->head != q->tail)
  {
    struct editor_cmd_t *cmd = q->data[q->head];

    switch (cmd->type)
    {
    case EDITOR_CMD_BRUSH_CREATE:
    {
      // TODO

      EditorCreate_Brush(arr,
                         (Vector){cmd->brush_create.startx, cmd->brush_create.starty, cmd->brush_create.startz},
                         (Vector){cmd->brush_create.endx, cmd->brush_create.endy, cmd->brush_create.endz},
                         (Vector){cmd->brush_create.sx, cmd->brush_create.sy, cmd->brush_create.sz},
                         cmd->brush_create.is_entity);
      break;
    }
    }
    q->head = (q->head + 1) % q->capacity;
  }
}
