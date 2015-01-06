#pragma once

#include <tgmath.h>
#include "image.h"
#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
  unsigned code;
  int width;
  int height;
  int bearingX;
  int bearingY;
  int advance;
  int textureIdx;
  int textureX;
  int textureY;
} graphics_Glyph;

typedef struct {
  graphics_Glyph const* glyphs;
  int numGlypths;

} graphics_GlyphSet;

typedef struct {
  GLuint* textures;

  graphics_GlyphSet glyphs[256];

  int numTextures;
  int currentX;
  int currentY;

} graphics_GlyphMap;

typedef struct {
  FT_Face face;
  graphics_GlyphMap glyphs;
} graphics_Font;


int graphics_Font_getWrap(graphics_Font const* font, char const* line, int width, char **wrapped);

void graphics_font_init();

int graphics_Font_new(graphics_Font *dst, char const* filename, int ptsize);

void graphics_Font_free(graphics_Font *obj);

int graphics_Font_getHeight(graphics_Font const* font);

int graphics_Font_getAscent(graphics_Font const* font);

int graphics_Font_getDescent(graphics_Font const* font);

int graphics_Font_getBaseline(graphics_Font const* font);

int graphics_Font_getWidth(graphics_Font const* font, char const* line);

typedef enum {
  graphics_TextAlign_center,
  graphics_TextAlign_left,
  graphics_TextAlign_right,
  graphcis_TextAlign_justify
} graphics_TextAlign;

void graphics_Font_render(graphics_Font* font, char const* text);