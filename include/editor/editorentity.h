#ifndef EDITOR_ENTITY_H
#define EDITOR_ENTITY_H

#include "types/types_vector.h"
#include "rendering/light.h"

#define EDITOR_ENTITY_LIMIT 1024

typedef enum editor_entity_type_t{
  EDITOR_ENTITY_PLAYER_SPAWN,
  EDITOR_ENTITY_LIGHT,
} editor_entity_type_t;

typedef struct editor_entity_t{

  editor_entity_type_t type;
  Vector pos;
  int id;

  union{
    struct {
      int spawn_order; // Order in the list of spawn points
      Vector direction; // Players spawn facing this direction
    } ent_playerspawn;

    struct {
      light_type_t light_type;
      Vector direction;
      Vector colour;
      float intensity;
      float radius;
    } ent_lightsource;
  };
} editor_entity_t;

typedef struct editor_entity_list_t {
  size_t capacity;
  size_t count;
  editor_entity_t* entities;
} editor_entity_list_t;

int EditorEntity_ListAdd(editor_entity_list_t* list, editor_entity_t entity);

#endif