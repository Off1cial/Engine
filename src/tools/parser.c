#include "tools/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* Parser_ReadFileText(const char *filepath)
{
  FILE *f = fopen(filepath, "rb");
  if (!f)
    return NULL;

  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  rewind(f);

  char *data = malloc(size + 1);

  fread(data, 1, size, f);
  data[size] = 0;

  fclose(f);
  printf("[PARSER]: Successfully read: %s\n", filepath);

  return data;
}

void Parser_SkipWhitespace(parser_t *p)
{
  while (*p->at && isspace(*p->at))
  {
    p->at++;
  }
}

void Parser_ReadIdentifier(parser_t *p, char *out)
{
  while (isalnum(*p->at) || *p->at == '_')
  {
    *out++ = *p->at++;
  }

  *out = 0;
}

void Parser_ReadString(parser_t *p, char *out)
{
  if (*p->at == '"')
  {
    p->at++;
  }

  while (*p->at && *p->at != '"')
  {
    *out++ = *p->at++;
  }

  if (*p->at == '"')
  {
    p->at++;
  }

  *out = 0;
}

int Parser_ReadBool(parser_t *p)
{
  if (strncmp(p->at, "true", 4) == 0)
  {
    p->at += 4;
    return 1;
  }

  if (strncmp(p->at, "false", 5) == 0)
  {
    p->at += 5;
    return 0;
  }

  return 0;
}


float Parser_ReadFloat(parser_t *p)
{
  Parser_SkipWhitespace(p);

  char buffer[64];
  char *out = buffer;

  // optional sign
  if (*p->at == '-' || *p->at == '+')
  {
    *out++ = *p->at++;
  }

  // integer part
  while (isdigit(*p->at))
  {
    *out++ = *p->at++;
  }

  // decimal part
  if (*p->at == '.')
  {
    *out++ = *p->at++;

    while (isdigit(*p->at))
    {
      *out++ = *p->at++;
    }
  }

  // exponent part
  if (*p->at == 'e' || *p->at == 'E')
  {
    *out++ = *p->at++;

    if (*p->at == '-' || *p->at == '+')
    {
      *out++ = *p->at++;
    }

    while (isdigit(*p->at))
    {
      *out++ = *p->at++;
    }
  }

  *out = 0;

  return strtof(buffer, NULL);
}
