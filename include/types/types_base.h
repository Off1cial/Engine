#ifndef TYPES_BASE_H
#define TYPES_BASE_H

#include <stdbool.h>

inline float BitsToFloat(unsigned int i){
  return *(float*)(&i);
}



#endif
