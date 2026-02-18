#include <stdio.h>
#include <glad/glad.h>
#include "application.h"
#include "rendering/mesh.h"
#include "inputbase.h"
#include "types/types_vector.h"

int main(){
  int app_running;
  struct container_t game_container;
  struct inputstate_t input_state;
  app_running = !ApplicationInit(&game_container);
 
  mesh_t triangle;
  MeshInit(&triangle, 3, 3);

  struct vertex_t v0 = {
    .pos = {-0.5f, 0.0f, 3.0f},
    .colour = {1.0f, 1.0f, 1.0f},
  };
  struct vertex_t v1 = {
    .pos = {-0.0f, 0.5f, 3.0f},
    .colour = {1.0f, 1.0f, 1.0f},
  };
  struct vertex_t v2 = {
    .pos = {0.5f, 0.0f, 3.0f},
    .colour = {1.0f, 1.0f, 1.0f},
  };

  MeshPushTriangle(
    &triangle,
    MeshPushVertex(&triangle, v0),
    MeshPushVertex(&triangle, v1),
    MeshPushVertex(&triangle, v2)
  );

  MeshUpload(&triangle,  GL_STATIC_DRAW);

  while(app_running){
    poll_input(&input_state, &app_running); 
    


    // Rendering
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
    MeshDraw(&triangle, GL_TRIANGLES);

    SDL_GL_SwapWindow(game_container.window);
  }

  ApplicationDestroy(&game_container);

  return 0;
}
