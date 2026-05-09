#ifndef TYPES_BASE_H
#define TYPES_BASE_H

#include <stdbool.h>
#include <string.h>

static inline float BitsToFloat(unsigned int i){
  float f;
  memcpy(&f, &i, sizeof(f));
  return f;
}



#endif
