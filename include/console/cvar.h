#ifndef CVAR_H
#define CVAR_H

#include <stdint.h>
#include <string.h>

#define CVAR_HASH_SIZE 256
#define CVAR_MAX_CVARS 256

static inline uint32_t fnv32_hash(const char *str, size_t len)
{
  unsigned char *s = (unsigned char *)str; /* unsigned string */

  /* See the FNV parameters at www.isthe.com/chongo/tech/comp/fnv/#FNV-param */
  const uint32_t FNV_32_PRIME = 0x01000193; /* 16777619 */

  uint32_t h = 0x811c9dc5; /* 2166136261 */
  while (len--)
  {
    /* xor the bottom with the current octet */
    h ^= *s++;
    /* multiply by the 32 bit FNV magic prime mod 2^32 */
    h *= FNV_32_PRIME;
  }

  return h;
}

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

typedef struct cvar_t cvar_t;

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

void Cvar_Register(cvar_t* var);

cvar_t* Cvar_Find(const char* name);


#endif