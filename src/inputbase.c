#include "inputbase.h"
#include "imgui_layer.h"
#include <SDL3/SDL.h>

void poll_input(struct inputstate_t* state, int* running_condition){
  
  // Copy keyboard state
  memcpy(state->kPrevious, state->kCurrent, sizeof(state->kPrevious));
  // Copy mouse state
  memcpy(state->mPrevious, state->mCurrent, sizeof(state->mPrevious));

  memset(state, 0, sizeof(*state));

  state->FLAG_WindowResized = false;
  SDL_Event event;
  while (SDL_PollEvent(&event)){
    ImGui_PassEvent(&event);
    if (event.type == SDL_EVENT_QUIT){
      *running_condition = 0;
    }
    if (event.type == SDL_EVENT_WINDOW_RESIZED){
      state->FLAG_WindowResized = true;
    }
  }

  const bool *keys = SDL_GetKeyboardState(NULL);
  memcpy(state->kCurrent, keys, sizeof(state->kCurrent));

  // Mouse control
  state->mx_rel = 0;
  state->my_rel = 0;
  SDL_MouseButtonFlags mbuttons = SDL_GetRelativeMouseState(&state->mx_rel, &state->my_rel);

  state->mbutton_left = (mbuttons & SDL_BUTTON_LMASK) != 0;
  state->mbutton_middle = (mbuttons & SDL_BUTTON_MMASK) != 0;
  state->mbutton_right = (mbuttons & SDL_BUTTON_RMASK) != 0;

  // Update camera

}
