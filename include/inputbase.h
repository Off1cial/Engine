#pragma once

#include <stdbool.h>

#define MAX_KEYS 512

struct inputstate_t{
  bool kCurrent[MAX_KEYS]; 
  bool kPrevious[MAX_KEYS]; 

  bool mCurrent[5];
  bool mPrevious[5];
};

void poll_input(struct inputstate_t* state, int* running_condition);
