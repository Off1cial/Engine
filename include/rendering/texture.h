#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>
#include <glad/glad.h>

#define MAX_TEXTURES 512

typedef struct {
  GLuint texid;
  int width, height;

  GLenum target;
  GLenum format;

  uint32_t flags;
  char debug_name[64];
} texture_t;

extern texture_t gTextures[MAX_TEXTURES];


void Texture_Bind(texture_t* texture, int slot);
texture_t TexutreLoad(const char* filepath);


#endif
