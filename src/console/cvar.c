#include "console/cvar.h"
cvar_reg_t gCvarRegistry;

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
