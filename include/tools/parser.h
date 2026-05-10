#ifndef PARSER_H
#define PARSER_H

typedef struct {
    const char* at;
} parser_t;

char* Parser_ReadFileText(const char* filepath);
void Parser_SkipWhitespace(parser_t* p);

void Parser_ReadIdentifier(parser_t* p, char* out);
void Parser_ReadString(parser_t* p, char* out);
int Parser_ReadBool(parser_t* p);

#endif