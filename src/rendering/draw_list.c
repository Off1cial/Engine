#include "rendering/draw_list.h"

/*

void RDrawList_Init(struct rdrawlist_t* list, size_t capacity){
  list->data = malloc(sizeof(struct rcmd_t) * capacity);
  list->capacity = capacity;
  list->count = 0;
}

void RDrawList_Add(struct rdrawlist_t* list, struct rcmd_t cmd){
  if (list->count >= list->capacity){
    size_t new_capacity = list->capacity * 2;
    struct rcmd_t* new_data = realloc(list->data, sizeof(struct rcmd_t)*new_capacity);

    if (!new_data){
      fprintf(stderr, "[RenderDrawList]: Failed to reallocate new data\n");
      exit(1);
    }
    list->data = new_data;
    list->capacity = new_capacity;
  }
  list->data[list->count++] = cmd;
}


void RDrawList_Clear(struct rdrawlist_t* list){
  list->count = 0;
}


void RDrawList_Execute(struct rdrawlist_t* list){
  size_t current = 0;
  while (current < list->count){
    struct rcmd_t cmd = list->data[current];
    switch(cmd.type){
      case RCMD_DRAW_MESH:{
        RCMD_DrawMesh(cmd);
        break;
      }
    }
  }
}

*/



void RDrawQueue_Init(rdrawqueue_t* q, size_t capacity){
  q->data = malloc(sizeof(struct rcmd_t*) * capacity);

  q->head = 0;
  q->tail = 0;
  q->capacity = capacity;
}

void RDrawQueue_Destroy(rdrawqueue_t* q){
  if (!q) return;

  free(q->data); q->data = NULL;
  q->capacity = q->head = q->tail = 0;
}

void RDrawQueue_Reset(rdrawqueue_t* q){
    q->head = 0;
    q->tail = 0;
}

void RDrawQueue_Push(rdrawqueue_t* q, struct rcmd_t* cmd){
  size_t next = (q->tail + 1) % q->capacity;
  if (next == q->head){
    fprintf(stderr, "[RDrawQueue]: Overflow!\n");
    exit(1);
  }
  q->data[q->tail] = cmd;
  q->tail = next;
}

void RDrawQueue_Execute(rdrawqueue_t* q){
  int count = 0;
  while (q->head != q->tail){
    struct rcmd_t* cmd = q->data[q->head];
    //printf("Executing command\n");
    switch(cmd->type){
      case RCMD_DRAW_MESH:{
        count++;
        R_DrawMesh(cmd);
        break;
      }
    }
    q->head = (q->head + 1) % q->capacity;
  }
  //printf("[RENDER]: %d commands executed\n", count);

}
