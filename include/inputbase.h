#pragma once

#include <stdbool.h>

#define MAX_KEYS 512

struct inputstate_t{
  bool kCurrent[MAX_KEYS]; 
  bool kPrevious[MAX_KEYS]; 

  bool mCurrent[5];
  bool mPrevious[5];
  bool mbutton_left, mbutton_middle, mbutton_right;
  bool FLAG_WindowResized;

  float mx_rel;
  float my_rel;
};

void poll_input(struct inputstate_t* state, int* running_condition);
