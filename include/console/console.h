#ifndef CONSOLE_H
#define CONSOLE_H

#include "console/cvar.h"
#include "console/concmd.h"


#define CONSOLE_MAX_LINES 32


typedef struct console_line_t{ 
  char text[256];
} console_line_t;

typedef struct console_t {
  console_line_t lines[CONSOLE_MAX_LINES];
  char input[256];
} console_t;


#endif