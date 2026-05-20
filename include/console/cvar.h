#ifndef CVAR_H
#define CVAR_H




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

typedef struct cvar_t
{

  cvar_valuetype_t vtype;

  char name[128];
  char desc[256];
  int ischeat;

  union
  {
    cvar_int_t i;
    cvar_float_t f;
  };
} cvar_t;

#endif