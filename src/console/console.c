#include "console/console.h"
#include "console/cvar.h"
#include "rendering/renderer.h"

#include "tools/flagtool.h"

console_t *gConsole = NULL;

cvar_t cvar_mat_fullbright = {
  .desc = "Int 0/1: Toggles the renderer's use of lighting",
  .ename = CVAR_NAME_MAT_FULLBRIGHT,
  .name = "mat_fullbright",
  .vtype = CVAR_TYPE_INT,
  .i.value = 0,
  .i.vdefault = 0,
  .i.vmin = 0,
  .i.vmax = 1
};

cvar_t cvar_mat_normals = {
  .desc = "Int 0/1: Toggles the renderer's use of normal maps",
  .ename = CVAR_NAME_MAT_NORMALS,
  .name = "mat_normals",
  .vtype = CVAR_TYPE_INT,
  .i.value = 1,
  .i.vdefault = 1,
  .i.vmin = 0,
  .i.vmax = 1
};

cvar_t cvar_r_wireframe = {
  .desc = "Int 0/1: Toggles the renderer's use of wireframes",
  .ename = CVAR_NAME_R_WIREFRAME,
  .name = "r_wireframe",
  .vtype = CVAR_TYPE_INT,
  .i.value = 0,
  .i.vdefault = 0,
  .i.vmin = 0,
  .i.vmax = 1
};

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL3/SDL.h>

void CVAR_InitAll()
{
  Cvar_Register(&cvar_mat_fullbright);
  Cvar_Register(&cvar_mat_normals);
  Cvar_Register(&cvar_r_wireframe);
}

static int console_push_line(console_line_t line)
{
  gConsole->lines[gConsole->line_tail] = line;
  gConsole->line_tail = (gConsole->line_tail + 1) % CONSOLE_MAX_LINES;

  // Queue full
  if (gConsole->line_head == gConsole->line_tail)
  {
    gConsole->line_head = (gConsole->line_head + 1) % CONSOLE_MAX_LINES;
  }
}

void Console_WriteLine(const char *text, console_line_type_t type)
{

  if (strlen(text) > CONSOLE_LINE_LENGTH)
  {
    fprintf(stderr, "[CONSOLE]: Warning, attempting to write a line longer than %d character(s) to console\n", CONSOLE_LINE_LENGTH);
    return;
  }

  console_line_t new_line = {0};
  new_line.type = type;
  strcpy(new_line.text, text);

  console_push_line(new_line);
}


static void console_output_file(const char* filepath){
  FILE* file = fopen(filepath, "r");
  if (!file){
    char msg[256];
    snprintf(msg, sizeof(msg), "Failed to open file: '%s'", filepath);
    Console_WriteLine(msg, CONSOLE_LINE_WARNING);
    return;
  }

  char line[256];
  while (fgets(line, sizeof(line), file)){
    line[strcspn(line, "\n")] = '\0';
    Console_WriteLine(line, CONSOLE_LINE_DEFAULT);
  }
  fclose(file);
}


void Console_Init()
{
  gConsole = malloc(sizeof(console_t));
  CVAR_InitAll();

  gConsole->line_head = 0;
  gConsole->line_tail = 0;
  gConsole->visible = 0;

  gConsole->input[0] = '\0';


  gConsole->parser.at = NULL;

  console_output_file("../Assets/chud.ascii");
}





void Cvar_Update(cvar_t* cvar){
  // Handle each cvar
  switch(cvar->ename){
    // mat_fullbright
    case CVAR_NAME_MAT_FULLBRIGHT:
      if (cvar->i.value == 1){
        //gRendererState->flags |= RENDERER_FLAG_FULLBRIGHT;
        SET_FLAG_MASK(gRendererState->flags, RENDERER_FLAG_FULLBRIGHT);
        //printf("Renderer flag set\n");
      }else{
        //gRendererState->flags &= ~RENDERER_FLAG_FULLBRIGHT;
        CLR_FLAG_MASK(gRendererState->flags, RENDERER_FLAG_FULLBRIGHT);
      }
      //Console_WriteLine("Updated mat_fullbright", CONSOLE_LINE_INPUT);
      break;

    // mat_normals
    case CVAR_NAME_MAT_NORMALS:
      if (cvar->i.value == 0){
        SET_FLAG_MASK(gRendererState->flags, RENDERER_FLAG_FLATTEXTURE);
      }else{
        CLR_FLAG_MASK(gRendererState->flags, RENDERER_FLAG_FLATTEXTURE);
      }
      break;

    // r_wireframe
    case CVAR_NAME_R_WIREFRAME:
      if (cvar->i.value == 1){
        SET_FLAG_MASK(gRendererState->flags, RENDERER_FLAG_WIREFRAME);
      }else{
        CLR_FLAG_MASK(gRendererState->flags, RENDERER_FLAG_WIREFRAME);
      }
      break;
  }
}



int Console_ParseInput(){
  gConsole->parser.at = gConsole->input;

  char c_name[256];
  Parser_ReadToken(&gConsole->parser, c_name);


  cvar_t* cvar = Cvar_Find(c_name);
  printf("cvar name: %s\n", c_name);
  // concmd_t* cmd = Concmd_Find(c_name);
  if (!cvar){ // && !cmd
    char err[256];
    snprintf(err, sizeof(err), "Unknown command: '%s'\n", c_name);
    Console_WriteLine(err, CONSOLE_LINE_WARNING);
    return 0;
  }

  if (cvar){
    switch(cvar->vtype){
      case CVAR_TYPE_INT:
        int new_val;
        char msg[256];
        if (!Parser_ReadInt(&gConsole->parser, &new_val)){
          // Failed to find value
          Console_WriteLine(cvar->desc, CONSOLE_LINE_DEFAULT);
          // Print current value
          snprintf(msg, sizeof(msg), "Current value: %d", cvar->i.value);
          Console_WriteLine(msg, CONSOLE_LINE_DEFAULT);
          break;
        }
        else{
          if (new_val < cvar->i.vmin){ new_val = cvar->i.vmin; }
          else if (new_val > cvar->i.vmax){ new_val = cvar->i.vmax; } 
          printf("CVAR: %s\nNew val: %d\n", cvar->name, new_val);
        }

        if (new_val != cvar->i.value){
          cvar->i.value = new_val;
          Cvar_Update(cvar);
        }
        break;
    }
  }
  return 1;
}



void Console_Execute(console_input_t input){

}





/* OLD
void Console_ProcessSDLevent(SDL_Event *event, SDL_Window *window)
{
  if (!gConsole->visible)
  {
    return;
  }

  switch (event->type)
  {
  case SDL_EVENT_TEXT_INPUT:
    strncat(gConsole->input, event->text.text, sizeof(gConsole->input) - strlen(gConsole->input) - 1);
    break;
  case SDL_EVENT_KEY_DOWN:
    switch (event->key.key)
    {
    case SDLK_BACKSPACE:
      size_t len = strlen(gConsole->input);

      if (len > 0)
      {
        gConsole->input[len - 1] = '\0';
      }
      break;
    case SDLK_RETURN:
      // Parse and execute
      printf("Break\n");
      Console_WriteLine(gConsole->input, CONSOLE_LINE_INPUT);

      cvar_t *cvar = Cvar_Find(gConsole->input);
      if (cvar)
      {
        printf("%s\n", cvar->desc);
      }
      else
      {
        printf("Cvar not found: %s\n", gConsole->input);
      }

      gConsole->input[0] = '\0';
      // SDL_StopTextInput(window);
      break;
    }

    break;
  }
}

*/
