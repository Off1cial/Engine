// rendering/text.c
#include "rendering/text.h"
#include "rendering/render_commands.h"
#include "rendering/renderer.h"
#include "rendering/mesh.h"
#include "mem.h"
#include <stdio.h>
#include <string.h>

font_t *Font_Load(const char *path, int fontSize)
{
  font_t *font = malloc(sizeof(font_t));
  if (!font)
    return NULL;
  memset(font, 0, sizeof(font_t));

  font->fontSize = fontSize;
  font->atlasSize = GLYPH_ATLAS_SIZE;

  // Open the font
  TTF_Font *ttf = TTF_OpenFont(path, (float)fontSize);
  if (!ttf)
  {
    printf("Failed to load font: %s\n", SDL_GetError());
    free(font);
    return NULL;
  }

  font->lineHeight = TTF_GetFontHeight(ttf);

  // Create a surface for the atlas (SDL3 initializes to transparent)
  SDL_Surface *atlas = SDL_CreateSurface(
      GLYPH_ATLAS_SIZE, GLYPH_ATLAS_SIZE,
      SDL_PIXELFORMAT_RGBA8888);
  if (!atlas)
  {
    printf("Failed to create font atlas surface: %s\n", SDL_GetError());
    TTF_CloseFont(ttf);
    free(font);
    return NULL;
  }

  printf(
      "font ascent=%d descent=%d line=%d\n",
      TTF_GetFontAscent(ttf),
      TTF_GetFontDescent(ttf),
      TTF_GetFontHeight(ttf)
  );

  int penX = 0, penY = 0;
  int maxRowHeight = 0;

  // Render printable ASCII (32-126)
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  for (int c = 32; c < 127 && c < MAX_GLYPHS; c++)
  {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *glyphSurface = TTF_RenderGlyph_Blended(ttf, (Uint32)c, white);
    if (!glyphSurface)
      continue;

    int gw = glyphSurface->w;
    int gh = glyphSurface->h;

    // Check if we need a new row
    if (penX + gw + 1 >= GLYPH_ATLAS_SIZE)
    {
      penX = 0;
      penY += maxRowHeight + 1;
      maxRowHeight = 0;
    }

    if (penY + gh >= GLYPH_ATLAS_SIZE)
    {
      printf("Font atlas overflow at character '%c' (%d)\n", (char)c, c);
      SDL_DestroySurface(glyphSurface);
      break;
    }

    // Copy glyph to atlas
    SDL_Rect dest = {penX, penY, gw, gh};
    SDL_BlitSurface(glyphSurface, NULL, atlas, &dest);

    // Get glyph metrics
    int advance = 0;
    int bearingX = 0, bearingY = 0;
    TTF_GetGlyphMetrics(ttf, (Uint32)c, NULL, NULL, &bearingX, &bearingY, &advance);
    font->ascent = TTF_GetFontAscent(ttf);


    // Store glyph info

    glyph_t *glyph = &font->glyphs[c];

    glyph->u0 = (float)penX / (float)GLYPH_ATLAS_SIZE;
    glyph->u1 = (float)(penX + gw) / (float)GLYPH_ATLAS_SIZE;
    glyph->v0 = (float)penY / (float)GLYPH_ATLAS_SIZE;        // Top of glyph in atlas
    glyph->v1 = (float)(penY + gh) / (float)GLYPH_ATLAS_SIZE; // Bottom of glyph in atlas

    glyph->w = gw;
    glyph->h = gh;
    glyph->bearingX = bearingX;
    glyph->bearingY = bearingY;
    glyph->advance = advance;

    

    penX += gw + 1;
    if (gh > maxRowHeight)
      maxRowHeight = gh;

    SDL_DestroySurface(glyphSurface);
  }

  // Create OpenGL texture from atlas
  SDL_SaveBMP(atlas, "../font_atlas.bmp");
  glGenTextures(1, &font->texture);
  glBindTexture(GL_TEXTURE_2D, font->texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GLYPH_ATLAS_SIZE, GLYPH_ATLAS_SIZE,
               0, GL_RGBA, GL_UNSIGNED_BYTE, atlas->pixels);
  GLenum err = glGetError();
  printf("tex upload error = %x\n", err);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  SDL_DestroySurface(atlas);
  TTF_CloseFont(ttf);

  printf("[FONTS]: Font loaded: %s (%dpx, %.0f line height)\n", path, fontSize, font->lineHeight);
  return font;
}

void Font_Destroy(font_t *font)
{
  if (!font)
    return;
  if (font->texture)
  {
    glDeleteTextures(1, &font->texture);
  }
  free(font);
}

const char *TextVector(Vector v)
{
  char *buf = MEM_ARENA_ALLOC(gMemArena, 64, 1);
  snprintf(buf, 64, "(%.1f, %.1f, %.1f)", v.x, v.y, v.z);
  return buf;
}