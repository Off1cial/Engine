#include "inputbase.h"
#include <SDL3/SDL.h>

void poll_input(struct inputstate_t* state, int* running_condition){
  SDL_Event event;
  while (SDL_PollEvent(&event)){
    if (event.type == SDL_EVENT_QUIT){
      *running_condition = 0;
    }
  }

}
