#include "console/console.h"


cvar_t cvar_mat_fullbright = {
  .desc = "Boolean: Toggles the renderer's use of lighting",
  .name = "mat_fullbright",
  .vtype = CVAR_TYPE_INT,
  .i.value = 0,
  .i.vdefault = 0,
  .i.vmin = 0,
  .i.vmax = 1
};

cvar_t cvar_mat_normals = {
  .desc = "Boolean: Toggles the renderer's use of normal maps",
  .name = "mat_normals",
  .vtype = CVAR_TYPE_INT,
  .i.value = 1,
  .i.vdefault = 1,
  .i.vmin = 0,
  .i.vmax = 1
};


void CVAR_InitAll(){
  Cvar_Register(&cvar_mat_fullbright);
  Cvar_Register(&cvar_mat_normals);
}