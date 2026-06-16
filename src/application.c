#include "application.h"
#include "headers.h"
#include <glad/glad.h>

struct container_t* APPLIACTION_CONTAINER = NULL;

void ApplicationDestroy(struct container_t* container){
  SDL_GL_DestroyContext(container->glContext);
  SDL_DestroyWindow(container->window);
}

int ApplicationInit(struct container_t* container, int winw, int winh){
  if (!SDL_Init(SDL_INIT_VIDEO)){
    fprintf(stderr, "Failed to start SDL3\n");
    exit(1);
  }
  container->window = NULL;
  container->glContext = NULL;

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);


  container->window = SDL_CreateWindow("Jay loves oiled black dudes", winw, winh,
                                       SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN
                                       );
  SDL_Surface* icon = SDL_LoadBMP("../Assets/icon.bmp");
  if (icon){
    SDL_SetWindowIcon(container->window, icon);
    SDL_DestroySurface(icon);
  }
  container->win_w = winw;
  container->win_h = winh;

  if (!container->window){
    fprintf(stderr, "Failed to create window\n");
    exit(1);
  }
  
  container->glContext = SDL_GL_CreateContext(container->window);
  if (!container->glContext){
    fprintf(stderr, "Failed to create OpenGL context\n");
    exit(1);
  }
  
  if(!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)){
    fprintf(stderr, "Bruh\n");
    exit(1);
  }
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
  printf("OpenGL version: %s\n", glGetString(GL_VERSION));
  printf("GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));



  return 0;
}
