#include "editor/editorentity.h"
#include "mem.h"

static int grow_entity_list(editor_entity_list_t* list){

  size_t old_size = 
    sizeof(editor_entity_t) * list->capacity;

  if (list->capacity * 2 >= EDITOR_ENTITY_LIMIT){
    list->capacity = EDITOR_ENTITY_LIMIT;
  }
  else{
    list->capacity *= 2;
  }

  size_t new_size = 
    sizeof(editor_entity_t) * list->capacity;

  if (new_size == old_size){
    // Resize didnt change anything
    return 0;
  }
  list->entities = ALIGNED_REALLOC(
    list->entities, 
    old_size, 
    new_size
  );
  return 1;
}

int EditorEntity_ListAdd(editor_entity_list_t* list, editor_entity_t entity){
  if (list->count >= list->capacity){
    int resize_success = grow_entity_list(list);
    if (!resize_success){
      return 0;
    }
  }

  list->entities[list->count++] = entity;
  return 1;
}