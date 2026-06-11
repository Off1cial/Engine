#ifndef INPUTBASE_H
#define INPUTBASE_H

#include <stdbool.h>
#include <SDL3/SDL.h>





#define MAX_KEYS 512

struct inputstate_t{
  bool kCurrent[MAX_KEYS]; 
  bool kPrevious[MAX_KEYS]; 

  bool mCurrent[5];
  bool mPrevious[5];
  bool mbutton_left, mbutton_middle, mbutton_right;
  bool mbutton_left_toggle, mbutton_right_toggle;
  bool mbutton_left_released, mbutton_right_released;

  bool FLAG_WindowResized;
  bool FLAG_ToggleEditor;

  float mx_rel;
  float my_rel;

  float mx, my;

  float scrl_y;
};

extern bool gCursorLocked;

#ifdef __cplusplus
extern "C" {
#endif

extern struct inputstate_t* gInputState;
extern bool gEditorGui_ViewportCaptured;

#ifdef __cplusplus
}
#endif




void poll_input(struct inputstate_t* state, int* running_condition, int* winw, int* winh, SDL_Window* window);

#endif