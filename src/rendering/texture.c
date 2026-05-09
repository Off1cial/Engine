#define STB_IMAGE_IMPLEMENTATION


#include "rendering/texture.h"
#include "rendering/stb_image.h"
#include <glad/glad.h>

texture_t gTextures[MAX_TEXTURES];
size_t texture_count = 0;

texture_t TextureLoad(const char* filepath){

  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);


  glTexParameteri(
    GL_TEXTURE_2D,
    GL_TEXTURE_WRAP_S,
    GL_REPEAT
  );
  glTexParameteri(
    GL_TEXTURE_2D,
    GL_TEXTURE_WRAP_T,
    GL_REPEAT
  );
  glTexParameteri(
    GL_TEXTURE_2D,
    GL_TEXTURE_MIN_FILTER,
    GL_LINEAR
  );

  int width, height, channels;
  
  unsigned char* pixels = stbi_load(filepath, &width, &height, &channels, 0);

  if (!pixels){
    fprintf(stderr, "[Texutre]: Failed to load texture: %s\n", filepath);
    exit(1);// No fucking clue if this even does anything
  }

  GLenum format = GL_RGB;
  if (channels == 1) format = GL_RED;
  else if (channels == 3) format = GL_RGB;
  else if (channels == 4) format = GL_RGBA;

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    format,
    width,
    height,
    0,
    format,
    GL_UNSIGNED_BYTE,
    pixels
  );
  glGenerateMipmap(GL_TEXTURE_2D);
  stbi_image_free(pixels);
  
  texture_t tex = {
    texture, width, height
  };

  gTextures[texture_count++] = tex;

  return tex;
}

void Texture_Bind(texture_t* texture, int slot){
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(texture->target, texture->texid);
}
