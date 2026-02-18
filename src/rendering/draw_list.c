#include "rendering/draw_list.h"

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


void RDrawList_Execute(struct rdrawlist_t* list){
  size_t current = 0;
  while (current < list->capacity){
    struct rcmd_t cmd = list->data[current];
    switch(cmd.type){
      case RCMD_DRAW_MESH:{
        RCMD_DrawMesh(cmd);
        break;
      }
    }
  }
}




