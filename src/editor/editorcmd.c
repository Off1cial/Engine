#include "editor/editorcmd.h"
#include <stdio.h>

void EditorQueue_Init(editor_cmd_queue_t* q, size_t capacity){
  q->data = malloc(sizeof(struct editor_cmd_t) * capacity);

  q->head = 0;
  q->tail = 0;
  q->capacity = capacity;
}

void EditorQueue_Destroy(editor_cmd_queue_t* q){
  if (!q) return;

  free(q->data); q->data = NULL;
  q->head = q->tail = q->capacity = 0;
}

void EditorQueue_Reset(editor_cmd_queue_t* q){
  q->head = 0;
  q->tail = 0;
}

void EditorQueue_Push(editor_cmd_queue_t* q, struct editor_cmd_t* cmd){
  size_t next = (q->tail + 1) % q->capacity;
  if (next == q->head){
    fprintf(stderr, "[EditorQueue]: Overflow!\n");
    exit(1);
  }
  q->data[q->tail] = cmd;
  q->tail = next;
}

void EditorQueue_Execute(editor_cmd_queue_t* q, brush_array_t* arr){
  while (q->head != q->tail){
    struct editor_cmd_t* cmd = q->data[q->head];

    switch(cmd->type){
      case EDITOR_CMD_BRUSH_CREATE:{
        // TODO
        EditorBrush_Create(
          arr, 
          (Vector){cmd->brush_create.px, cmd->brush_create.py, cmd->brush_create.pz},
          (Vector){cmd->brush_create.sx, cmd->brush_create.sy, cmd->brush_create.sz}
        );
        break;
      }
    }
    q->head = (q->head + 1) % q->capacity;
  }
}
