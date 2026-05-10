#define STB_IMAGE_IMPLEMENTATION

#include "rendering/texture.h"
#include "rendering/stb_image.h"

#include <glad/glad.h>

#include <stdio.h>

texture_t gTextures[MAX_TEXTURES];
size_t texture_count = 0;

texture_t *Texture_Load(const char *filepath)
{

  if (texture_count >= MAX_TEXTURES)
  {
    fprintf(stderr, "[Texture]: MAX_TEXTURES exceeded\n");
    return NULL;
  }

  int width, height, channels;

  unsigned char *pixels =
      stbi_load(filepath, &width, &height, &channels, 0);

  if (!pixels)
  {
    fprintf(stderr,
            "[Texture]: Failed to load texture: %s\n",
            filepath);

    return NULL;
  }

  GLuint texture;

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(
      GL_TEXTURE_2D,
      GL_TEXTURE_WRAP_S,
      GL_REPEAT);

  glTexParameteri(
      GL_TEXTURE_2D,
      GL_TEXTURE_WRAP_T,
      GL_REPEAT);

  glTexParameteri(
      GL_TEXTURE_2D,
      GL_TEXTURE_MIN_FILTER,
      GL_LINEAR_MIPMAP_LINEAR);

  glTexParameteri(
      GL_TEXTURE_2D,
      GL_TEXTURE_MAG_FILTER,
      GL_LINEAR);

  GLenum format = GL_RGB;
  GLenum internal = GL_RGB8;

  switch (channels)
  {

  case 1:
    format = GL_RED;
    internal = GL_R8;
    break;

  case 3:
    format = GL_RGB;
    internal = GL_RGB8;
    break;

  case 4:
    format = GL_RGBA;
    internal = GL_RGBA8;
    break;

  default:
    fprintf(stderr,
            "[Texture]: Unsupported channel count: %d\n",
            channels);

    stbi_image_free(pixels);

    return NULL;
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexImage2D(
      GL_TEXTURE_2D,
      0,
      internal,
      width,
      height,
      0,
      format,
      GL_UNSIGNED_BYTE,
      pixels);

  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(pixels);

  texture_t tex = {0};

  tex.texid = texture;
  tex.width = width;
  tex.height = height;
  tex.target = GL_TEXTURE_2D;
  tex.format = format;

  snprintf(
      tex.debug_name,
      sizeof(tex.debug_name),
      "%s",
      filepath);

  gTextures[texture_count] = tex;
  
  printf("[TEXTURE]: Successfully read texture: %s\n", filepath);
  return &gTextures[texture_count++];
}

void Texture_Bind(texture_t *texture, int slot)
{

  if (!texture)
  {
    return;
  }

  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(texture->target, texture->texid);
}