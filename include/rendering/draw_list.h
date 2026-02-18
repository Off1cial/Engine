#ifndef RENDERERING_DRAW_LIST_H
#define RENDERERING_DRAW_LIST_H

#include "rendering/render_commands.h"


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


#endif
