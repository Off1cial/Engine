#ifndef CVAR_H
#define CVAR_H

#include <stdint.h>

#define CVAR_HASH_SIZE 256
#define CVAR_MAX_CVARS 256

typedef enum cvar_flags_t
{
  CVAR_NONE      = 0,
  CVAR_CHEAT     = 1 << 0,
  CVAR_READONLY  = 1 << 1,
  CVAR_ARCHIVE   = 1 << 2,
} cvar_flags_t;

typedef enum cvar_valuetype_t
{
  CVAR_TYPE_INT,
  CVAR_TYPE_FLOAT,
  CVAR_TYPE_STRING
} cvar_valuetype_t;

typedef struct
{
  float value;
  float vdefault;
  float vmin;
  float vmax;
} cvar_float_t;

typedef struct
{
  int value;
  int vdefault;
  int vmin;
  int vmax;
} cvar_int_t;

typedef struct cvar_def_t{
  const char* name;
  const char* desc;
} cvar_def_t;

typedef struct cvar_t
{

  uint32_t hash;
  cvar_t* next;
  cvar_valuetype_t vtype;

  const char* name;
  const char* desc;
  uint32_t flags;

  union
  {
    cvar_int_t i;
    cvar_float_t f;
  };
} cvar_t;


typedef struct cvar_reg_t{
  cvar_t* reg[CVAR_MAX_CVARS];
} cvar_reg_t;

extern cvar_reg_t gCvarRegistry;

void Cvar_Register(cvar_t* var)
{
  var->hash = fnv32_hash(var->name, strlen(var->name));

  uint32_t bucket = var->hash % CVAR_HASH_SIZE;

  var->next = gCvarRegistry.reg[bucket];
  gCvarRegistry.reg[bucket] = var;
}

cvar_t* Cvar_Find(const char* name)
{
  uint32_t hash = fnv32_hash(name, strlen(name));

  uint32_t bucket = hash % CVAR_HASH_SIZE;

  cvar_t* var = gCvarRegistry.reg[bucket];

  while (var)
  {
    if (var->hash == hash &&
        strcmp(var->name, name) == 0)
    {
      return var;
    }

    var = var->next;
  }

  return NULL;
}


#endif