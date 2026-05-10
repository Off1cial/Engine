#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>
#include <stddef.h>

#include <glad/glad.h>

#define MAX_TEXTURES 512

typedef struct
{

  GLuint texid;

  int width;
  int height;

  GLenum target;
  GLenum format;

  uint32_t flags;

  char debug_name[64];

} texture_t;

extern texture_t gTextures[MAX_TEXTURES];
extern size_t texture_count;

texture_t *Texture_Load(const char *filepath);

void Texture_Bind(texture_t *texture, int slot);

#endif