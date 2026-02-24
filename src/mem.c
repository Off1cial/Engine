#include "mem.h"


int mem_validity(void* mem){
  if (NULL == mem){
    fprintf(stderr, "Invalid Memory\n");
    return 0;
  }
  return 1;
}

