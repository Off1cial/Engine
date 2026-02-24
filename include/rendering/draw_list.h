#ifndef RENDERERING_DRAW_LIST_H
#define RENDERERING_DRAW_LIST_H

#include "rendering/render_commands.h"

/*

struct rdraw_sortable_t{
  struct rcmd_t cmd;
  uint64_t key;
};


struct rdrawlist_t{
  struct rcmd_t* data;
  size_t capacity, count;
};

void RDrawList_Init(struct rdrawlist_t* list, size_t capacity);
void RDrawList_Add(struct rdrawlist_t* list, struct rcmd_t cmd);
void RDrawList_Execute(struct rdrawlist_t* list);
void RDrawList_Clear(struct rdrawlist_t* list);

*/

typedef struct{
  struct rcmd_t** data;
  size_t head, tail, capacity;
} rdrawqueue_t;

void RDrawQueue_Init(rdrawqueue_t* q, size_t capacity);
void RDrawQueue_Destroy(rdrawqueue_t* q);
void RDrawQueue_Reset(rdrawqueue_t* q);
void RDrawQueue_Push(rdrawqueue_t* q, struct rcmd_t* cmd);
void RDrawQueue_Execute(rdrawqueue_t* q);


#endif
