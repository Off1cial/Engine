#include "console/console.h"
#include "console/cvar.h"

console_t* gConsole = NULL;

cvar_t cvar_mat_fullbright = {
  .desc = "Boolean: Toggles the renderer's use of lighting",
  .name = "mat_fullbright",
  .vtype = CVAR_TYPE_INT,
  .i.value = 0,
  .i.vdefault = 0,
  .i.vmin = 0,
  .i.vmax = 1
};

cvar_t cvar_mat_normals = {
  .desc = "Boolean: Toggles the renderer's use of normal maps",
  .name = "mat_normals",
  .vtype = CVAR_TYPE_INT,
  .i.value = 1,
  .i.vdefault = 1,
  .i.vmin = 0,
  .i.vmax = 1
};

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL3/SDL.h>

void CVAR_InitAll(){
  Cvar_Register(&cvar_mat_fullbright);
  Cvar_Register(&cvar_mat_normals);
}


void Console_Init(){
  gConsole = malloc(sizeof(console_t));
  CVAR_InitAll();

  gConsole->line_head = 0;
  gConsole->line_tail = 0;
  gConsole->visible = 0;

  gConsole->input[0] = '\0';
}



static int console_push_line(console_line_t line){
  gConsole->lines[gConsole->line_tail] = line;
  gConsole->line_tail = (gConsole->line_tail + 1) % CONSOLE_MAX_LINES;

  // Queue full
  if (gConsole->line_head == gConsole->line_tail){
    gConsole->line_head = (gConsole->line_head + 1) % CONSOLE_MAX_LINES;
  }
}

void Console_WriteLine(const char* text, console_line_type_t type){

  if (strlen(text) > CONSOLE_LINE_LENGTH){
    fprintf(stderr, "[CONSOLE]: Warning, attempting to write a line longer than %d character(s) to console\n", CONSOLE_LINE_LENGTH);
    return;
  }


  console_line_t new_line = {0};
  new_line.type = type;
  strcpy(new_line.text, text);

  console_push_line(new_line);
}

void Console_ProcessSDLevent(SDL_Event* event, SDL_Window* window){
  if (!gConsole->visible){
    return;
  }

  

  switch(event->type){
    case SDL_EVENT_TEXT_INPUT:
      strncat(gConsole->input, event->text.text,sizeof(gConsole->input) - strlen(gConsole->input) - 1);
      break;
    case SDL_EVENT_KEY_DOWN:
      if (event->key.scancode == SDL_SCANCODE_BACKSPACE){
        size_t len = strlen(gConsole->input);

        if (len > 0){
          gConsole->input[len - 1] = '\0';
        }
      }
      else if (event->key.scancode == SDL_SCANCODE_RETURN){
        // Parse and execute
        Console_WriteLine(gConsole->input, CONSOLE_LINE_INPUT);
        gConsole->input[0] = '\0';
        //SDL_StopTextInput(window);
      }
    break;
  }

}

