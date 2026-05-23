#include "inputbase.h"
#include "imgui_layer.h"
#include "rendering/renderer.h"
#include "console/console.h"
#include <stdio.h>
#include <SDL3/SDL.h>




bool mleft_last = false;
bool mright_last = false;

void poll_input(struct inputstate_t* state, int* running_condition, int* winw, int* winh, SDL_Window* window){
  
  // Copy keyboard state
  memcpy(state->kPrevious, state->kCurrent, sizeof(state->kPrevious));
  // Copy mouse state
  memcpy(state->mPrevious, state->mCurrent, sizeof(state->mPrevious));

  mleft_last = state->mbutton_left;
  mright_last = state->mbutton_right;

  state->FLAG_WindowResized = false;
  state->FLAG_ToggleEditor = false;
  state->mbutton_left_toggle = false;
  state->mbutton_right_toggle = false;
  state->my_rel = 0;
  state->mx_rel = 0;
  state->scrl_y = 0.0f;

  SDL_Event event;
  while (SDL_PollEvent(&event)){
    ImGui_PassEvent(&event);
    //Console_ProcessSDLevent(&event, window); OLD - CONSOLE INPUT NOW HANDLED IMGUI
    if (event.type == SDL_EVENT_QUIT){
      *running_condition = 0;
    }
    if (event.type == SDL_EVENT_WINDOW_RESIZED){
      state->FLAG_WindowResized = true;
      *winw = event.window.data1;
      *winh = event.window.data2;
    }
    if (event.type == SDL_EVENT_MOUSE_WHEEL){
      state->scrl_y = event.wheel.y;
    }
  }

  const bool *keys = SDL_GetKeyboardState(NULL);
  memcpy(state->kCurrent, keys, sizeof(state->kCurrent));

  // Mouse control
  state->mx_rel = 0;
  state->my_rel = 0;
  SDL_GetMouseState(&state->mx, &state->my);
  SDL_MouseButtonFlags mbuttons = SDL_GetRelativeMouseState(&state->mx_rel, &state->my_rel);

  state->mbutton_left = (mbuttons & SDL_BUTTON_LMASK) != 0;
  state->mbutton_middle = (mbuttons & SDL_BUTTON_MMASK) != 0;
  state->mbutton_right = (mbuttons & SDL_BUTTON_RMASK) != 0;

  state->mbutton_left_released = (!state->mbutton_left) && (mleft_last);
  state->mbutton_right_released = (!state->mbutton_right) && (mright_last);

  if (state->mbutton_left  && !mleft_last){
    state->mbutton_left_toggle = true;
    printf("M1 toggle\n");
  }
  if (state->mbutton_right != mright_last){
    state->mbutton_right_toggle = true;
  }

  // Toggle editor
  state->FLAG_ToggleEditor = state->kCurrent[SDL_SCANCODE_0] && !state->kPrevious[SDL_SCANCODE_0];

  if (state->kCurrent[SDL_SCANCODE_8] && !state->kPrevious[SDL_SCANCODE_8]){
    gRendererState->wireframe = !gRendererState->wireframe;
  }

  if (state->kCurrent[SDL_SCANCODE_7] && !state->kPrevious[SDL_SCANCODE_7]){
    gRendererState->draw_normal_maps = !gRendererState->draw_normal_maps;
  }

  if (state->kCurrent[SDL_SCANCODE_GRAVE] && !state->kPrevious[SDL_SCANCODE_GRAVE]){
    gConsole->visible = !gConsole->visible;
    if (gConsole->visible){
      SDL_StartTextInput(window);
    }else{
      SDL_StopTextInput(window);
    }
  }
}
