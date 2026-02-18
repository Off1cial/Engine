#pragma once

#include "headers.h"

struct debug_logger_t{
  size_t log_count;
  FILE* current_file;
};

extern debug_logger_t gDebugLogger;

FILE* DebugLog_CreateFile();
FILE* DebugLog_GetCurrentFile();

void DebugLog_WriteToFile(FILE* fPtr, const char* text);


