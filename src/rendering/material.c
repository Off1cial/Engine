#include "rendering/material.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

static void Parser_SkipWhitespace(parser_t* p){
    while (*p->at && isspace(*p->at))
    {
        p->at++;
    }
}

static token_t Parser_ReadIdentifier(parser_t* p){
    token_t tok = {0};

    tok.type = TOKEN_IDENTIFIER;

    char* out = tok.text;

    while (isalnum(*p->at) || *p->at == '_'){
        *out++ = *p->at++;
    }

    *out = 0;

    return tok;
}

static token_t Parser_ReadString(parser_t* p){
    token_t tok = {0};

    tok.type = TOKEN_STRING;

    p->at++; // skip opening quote

    char* out = tok.text;

    while (*p->at && *p->at != '"'){
        *out++ = *p->at++;
    }

    *out = 0;

    if (*p->at == '"')
        p->at++;

    return tok;
}

static token_t Parser_NextToken(parser_t* p){
    Parser_SkipWhitespace(p);

    token_t tok = {0};

    char c = *p->at;

    if (!c){
        tok.type = TOKEN_EOF;
        return tok;
    }

    if (isalpha(c))
        return Parser_ReadIdentifier(p);

    if (c == '"')
        return Parser_ReadString(p);

    p->at++;

    switch (c){
        case '=':
            tok.type = TOKEN_EQUALS;
            break;

        case '{':
            tok.type = TOKEN_LBRACE;
            break;

        case '}':
            tok.type = TOKEN_RBRACE;
            break;
    }

    return tok;
}

material_t* Material_Load(const char* filepath){
  material_t* m = malloc(sizeof(material_t));
  if (!m){
    fprintf(stderr, "[MATERIAL]: Unable to load material: %s\n", filepath);
    return NULL;
  }
  
  // Fill m with data from file



  return m;
}
