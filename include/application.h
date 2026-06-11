#ifndef APPLICATION_H
#define APPLICATION_H

#include <SDL3/SDL.h>
#include <assert.h>

struct container_t{
  SDL_Window* window;
  int win_w;
  int win_h;
  char* title;
  SDL_GLContext glContext;
};

extern struct container_t* APPLIACTION_CONTAINER;


void ApplicationDestroy(struct container_t* container);

int ApplicationInit(struct container_t* container, int winw, int winh);


#endif
