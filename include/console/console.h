#ifndef CONSOLE_H
#define CONSOLE_H

#include "console/cvar.h"
#include "console/concmd.h"
#include "tools/parser.h"
#include <SDL3/SDL.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum console_line_type_t
{
  CONSOLE_LINE_DEFAULT,
  CONSOLE_LINE_WARNING,
  CONSOLE_LINE_INPUT
} console_line_type_t;

#define CONSOLE_MAX_LINES 32
#define CONSOLE_LINE_LENGTH 256

typedef struct console_line_t
{
  console_line_type_t type;
  char text[CONSOLE_LINE_LENGTH];
} console_line_t;

#define CONSOLE_MAX_HISTORY 64

typedef struct console_t
{
  bool visible;

  char input[256];

  char history[CONSOLE_MAX_HISTORY][256];

  int history_count;
  int history_pos;

  console_line_t lines[CONSOLE_MAX_LINES];

  int line_head;
  int line_tail;

  parser_t parser;

} console_t;

typedef enum console_input_type_t{
  CONSOLE_INPUT_CVAR,
  CONSOLE_INPUT_CMD
} console_input_type_t;

typedef struct console_input_t{
  console_input_type_t type;

  union{
    cvar_t* cvar;
    concmd_t* cmd;
  };
} console_input_t;

extern console_t *gConsole;

void Console_Init();
//void Console_ProcessSDLevent(SDL_Event *event, SDL_Window *window);

int Console_ParseInput();


void Console_WriteLine(const char *text, console_line_type_t type);

#ifdef __cplusplus
}
#endif

#endif