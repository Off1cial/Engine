#ifndef CONCMD_H
#define CONCMD_H


typedef struct concmd_t{
  char name[128];
  void (*fn)(int argc, char** argv);
} concmd_t;

#endif