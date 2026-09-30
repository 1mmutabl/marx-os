#pragma once

#include "types.h"
#include "vector.h"

#define COLOR_BLACK (color){ 0, 0, 0, 255 }
#define COLOR_WHITE (color){ 255, 255, 255, 255 }

#define COLOR_RED (color){ 255, 0, 0, 255 }
#define COLOR_GREEN (color){ 0, 255, 0, 255 }
#define COLOR_BLUE (color){ 0, 0, 255, 255 }

#define COLOR_YELLOW (color){ 255, 255, 0, 255 }
#define COLOR_CYAN (color){ 0, 255, 255, 255 }
#define COLOR_MAGENTA (color){ 255, 0, 255, 255 }

#define COLOR_ORANGE (color){ 255, 165, 0, 255 }
#define COLOR_PURPLE (color){ 128, 0, 128, 255 }
#define COLOR_PINK (color){ 255, 192, 203, 255 }
#define COLOR_BROWN (color){ 165, 42, 42, 255 }

#define COLOR_GRAY (color){ 128, 128, 128, 255 }
#define COLOR_DARK_GRAY (color){ 64, 64, 64, 255 }
#define COLOR_LIGHT_GRAY (color){ 192, 192, 192, 255 }

#define COLOR_DARK_RED (color){ 128, 0, 0, 255 }
#define COLOR_DARK_GREEN (color){ 0, 128, 0, 255 }
#define COLOR_DARK_BLUE (color){ 0, 0, 128, 255 }

#define COLOR_LIGHT_RED (color){ 255, 128, 128, 255 }
#define COLOR_LIGHT_GREEN (color){ 128, 255, 128, 255 }
#define COLOR_LIGHT_BLUE (color){ 128, 128, 255, 255 }

typedef struct
{
  unsigned int Address;
  unsigned int Pitch;
  unsigned int Width;
  unsigned int Height;
} framebuffer;

typedef struct
{
  uint8 r;
  uint8 g;
  uint8 b;
  uint8 a;
} color;

typedef struct
{
  uVector Position;
  uVector Size;
  color   Color;
} rect;

typedef struct
{
  uVector Position;
  float   Scale;
  char    Character;
  color   Color;
} character;

typedef struct
{
  uVector     Position;
  float       Scale;
  const char *String;
  color       Color;

  int LineSpacing;
  int TabWidth;
} string;

typedef struct
{
  uVector Position;
  uVector Size;
  uVector SourceSize;
  color  *Pixels;

  unsigned int Channels;
} raw_pixels;

void clear_screen(framebuffer *Framebuffer, color Color);
void draw_rect(framebuffer *Framebuffer, rect *Rectangle);
void draw_char(framebuffer *Framebuffer, character *Character);
void draw_string(framebuffer *Framebuffer, string *String);
void draw_raw(framebuffer *Framebuffer, raw_pixels *Raw);

int     get_string_width(string String);
int     get_string_height(string String);
uVector get_string_size(string String);

void         graphics_init(framebuffer *RealFramebuffer,
                           unsigned int BackbufferAddress);
framebuffer *get_backbuffer(void);
void         end_drawing(void);

unsigned int color_to_pixel(color Color);
color        pixel_to_color(uint32 Color);
