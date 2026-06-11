// rendering/text.h
#ifndef TEXT_H
#define TEXT_H

#include <SDL3_ttf/SDL_ttf.h>
#include <glad/glad.h>
#include "types/types_vector.h"

#define GLYPH_ATLAS_SIZE 512
#define MAX_GLYPHS 128

typedef struct {
    float u0, v0;   // top-left UV
    float u1, v1;   // bottom-right UV
    int   w, h;     // pixel size
    int   bearingX; // offset from origin to left of glyph
    int   bearingY; // offset from baseline to top of glyph
    int   advance;  // how far to move right after this character
} glyph_t;

typedef struct {
  GLuint  texture;
  glyph_t glyphs[MAX_GLYPHS];
  int     atlasSize;
  int     fontSize;
  float   lineHeight;
} font_t;

// Font loading
font_t* Font_Load(const char *path, int fontSize);

// Text rendering (batched quads)
//void    Text_DrawString3D(font_t *font, const char *text, Vector origin, Vector right, Vector up, float scale, Vector4 colour);

// Cleanup
void    Font_Destroy(font_t *font);

#endif