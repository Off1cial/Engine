#ifndef PARSER_H
#define PARSER_H

typedef struct
{
  const char *at;
} parser_t;


#include <stdint.h>
static inline uint32_t fnv32_hash(const char *str, size_t len)
{
  unsigned char *s = (unsigned char *)str; /* unsigned string */

  /* See the FNV parameters at www.isthe.com/chongo/tech/comp/fnv/#FNV-param */
  const uint32_t FNV_32_PRIME = 0x01000193; /* 16777619 */

  uint32_t h = 0x811c9dc5; /* 2166136261 */
  while (len--)
  {
    /* xor the bottom with the current octet */
    h ^= *s++;
    /* multiply by the 32 bit FNV magic prime mod 2^32 */
    h *= FNV_32_PRIME;
  }

  return h;
}

char *Parser_ReadFileText(const char *filepath);
void Parser_SkipWhitespace(parser_t *p);

void Parser_ReadIdentifier(parser_t *p, char *out);
void Parser_ReadString(parser_t *p, char *out);
int Parser_ReadBool(parser_t *p);
float Parser_ReadFloat(parser_t *p);

#endif
