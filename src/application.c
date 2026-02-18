#include "application.h"
#include "headers.h"
#include <glad/glad.h>

void ApplicationDestroy(struct container_t* container){
  SDL_GL_DestroyContext(container->glContext);
  SDL_DestroyWindow(container->window);
}

int ApplicationInit(struct container_t* container){
  if (!SDL_Init(SDL_INIT_VIDEO)){
    fprintf(stderr, "Failed to start SDL3\n");
    exit(1);
  }
  container->window = NULL;
  container->glContext = NULL;

  container->window = SDL_CreateWindow("Window", 640, 480,
                                       SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
                                       );

  if (!container->window){
    fprintf(stderr, "Failed to create window\n");
    exit(1);
  }
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  
  container->glContext = SDL_GL_CreateContext(container->window);
  if (!container->glContext){
    fprintf(stderr, "Failed to create OpenGL context\n");
    exit(1);
  }
  
  gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
  glEnable(GL_DEPTH_TEST);



  return 0;
}
