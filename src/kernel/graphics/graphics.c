#include "graphics.h"

#include "font.h"
#include "vector.h"

static inline void memset32(void *Dst, unsigned int Value, unsigned int Count)
{
  __asm__ volatile("rep stosl"
                   : "+D"(Dst), "+c"(Count)
                   : "a"(Value)
                   : "memory");
}

static inline void memcpy32(void *Dst, const void *Src, unsigned int Count)
{
  __asm__ volatile("rep movsl" : "+D"(Dst), "+S"(Src) : "c"(Count) : "memory");
}

static inline unsigned int blend_pixel(unsigned int DstPixel, color Src)
{
  if (Src.a == 0)
    return DstPixel;

  if (Src.a == 255)
    return color_to_pixel(Src);

  color Dst = pixel_to_color(DstPixel);

  unsigned int A    = Src.a;
  unsigned int InvA = 255 - A;

  Dst.r = (uint8)(((unsigned int)Src.r * A + (unsigned int)Dst.r * InvA + 127) /
                  255);

  Dst.g = (uint8)(((unsigned int)Src.g * A + (unsigned int)Dst.g * InvA + 127) /
                  255);

  Dst.b = (uint8)(((unsigned int)Src.b * A + (unsigned int)Dst.b * InvA + 127) /
                  255);

  return color_to_pixel(Dst);
}

static inline void put_pixel(framebuffer Buffer, int x, int y, color Color)
{
  if (x < 0 || y < 0 || (unsigned int)x >= Buffer.Width ||
      (unsigned int)y >= Buffer.Height)
    return;

  unsigned int *Pixel =
      (unsigned int *)(Buffer.Address + y * Buffer.Pitch + x * 4);

  *Pixel = blend_pixel(*Pixel, Color);
}

unsigned int color_to_pixel(color Color)
{
  return ((unsigned int)Color.r << 16) | ((unsigned int)Color.g << 8) |
         (unsigned int)Color.b;
}

color pixel_to_color(uint32 Color)
{
  color Col;

  Col.r = (Color >> 16) & 0xFF;
  Col.g = (Color >> 8) & 0xFF;
  Col.b = Color & 0xFF;
  Col.a = 255;

  return Col;
}

void clear_screen(framebuffer Buffer, color Color)
{
  unsigned int PixelColor = color_to_pixel(Color);
  unsigned int Count      = (Buffer.Pitch * Buffer.Height) / 4;

  memset32((void *)Buffer.Address, PixelColor, Count);
}

void draw_rect_fill(framebuffer Buffer, rect Rectangle)
{
  if (Rectangle.Color.a == 0)
    return;

  int x0 = (int)Rectangle.Position.x;
  int y0 = (int)Rectangle.Position.y;

  int x1 = x0 + (int)Rectangle.Size.x;
  int y1 = y0 + (int)Rectangle.Size.y;

  for (int y = y0; y < y1; y++)
  {
    for (int x = x0; x < x1; x++)
      put_pixel(Buffer, x, y, Rectangle.Color);
  }
}

void draw_rect_hollow(framebuffer Buffer, rect_hollow Rectangle)
{
  if (Rectangle.Color.a == 0)
    return;

  int x0 = (int)Rectangle.Position.x;
  int y0 = (int)Rectangle.Position.y;

  int x1 = x0 + (int)Rectangle.Size.x;
  int y1 = y0 + (int)Rectangle.Size.y;

  int Thickness = (int)Rectangle.Thickness;

  if (Thickness <= 0)
    Thickness = 1;

  for (int y = y0; y < y1; y++)
  {
    for (int x = x0; x < x1; x++)
    {
      int FromLeft   = x - x0;
      int FromRight  = x1 - 1 - x;
      int FromTop    = y - y0;
      int FromBottom = y1 - 1 - y;

      if (FromLeft < Thickness || FromRight < Thickness ||
          FromTop < Thickness || FromBottom < Thickness)
        put_pixel(Buffer, x, y, Rectangle.Color);
    }
  }
}

void draw_line(framebuffer Buffer, line Line)
{
  if (Line.Color.a == 0)
    return;

  int x0 = (int)Line.Start.x;
  int y0 = (int)Line.Start.y;
  int x1 = (int)Line.End.x;
  int y1 = (int)Line.End.y;

  int dx = x1 - x0;
  int dy = y1 - y0;

  int sx = dx >= 0 ? 1 : -1;
  int sy = dy >= 0 ? 1 : -1;

  dx = dx >= 0 ? dx : -dx;
  dy = dy >= 0 ? dy : -dy;

  int err = dx - dy;

  float Thickness = Line.Thickness;

  if (Thickness <= 0.0f)
    Thickness = 1.0f;

  int Radius = (int)(Thickness * 0.5f);

  for (;;)
  {
    for (int oy = -Radius; oy <= Radius; oy++)
    {
      for (int ox = -Radius; ox <= Radius; ox++)
      {
        put_pixel(Buffer, x0 + ox, y0 + oy, Line.Color);
      }
    }

    if (x0 == x1 && y0 == y1)
      break;

    int e2 = 2 * err;

    if (e2 > -dy)
    {
      err -= dy;
      x0 += sx;
    }

    if (e2 < dx)
    {
      err += dx;
      y0 += sy;
    }
  }
}

void draw_circle_fill(framebuffer Buffer, circle Circle)
{
  if (Circle.Color.a == 0)
    return;

  int cx = (int)Circle.Center.x;
  int cy = (int)Circle.Center.y;
  int r  = (int)Circle.Radius;

  if (r <= 0)
    return;

  int r2 = r * r;

  for (int y = -r; y <= r; y++)
  {
    for (int x = -r; x <= r; x++)
    {
      if (x * x + y * y > r2)
        continue;

      put_pixel(Buffer, cx + x, cy + y, Circle.Color);
    }
  }
}

void draw_circle_hollow(framebuffer Buffer, circle_hollow Circle)
{
  if (Circle.Color.a == 0)
    return;

  int cx = (int)Circle.Center.x;
  int cy = (int)Circle.Center.y;

  int Radius    = (int)Circle.Radius;
  int Thickness = (int)Circle.Thickness;

  if (Radius <= 0)
    return;

  if (Thickness <= 0)
    Thickness = 1;

  if (Thickness > Radius)
    Thickness = Radius;

  int OuterRadius = Radius;
  int InnerRadius = Radius - Thickness;

  int OuterRadius2 = OuterRadius * OuterRadius;
  int InnerRadius2 = InnerRadius * InnerRadius;

  for (int y = -OuterRadius; y <= OuterRadius; y++)
  {
    for (int x = -OuterRadius; x <= OuterRadius; x++)
    {
      int Distance2 = x * x + y * y;

      if (Distance2 > OuterRadius2)
        continue;

      if (Distance2 < InnerRadius2)
        continue;

      put_pixel(Buffer, cx + x, cy + y, Circle.Color);
    }
  }
}

void draw_char(framebuffer Buffer, character Character)
{
  if (Character.Color.a == 0)
    return;

  char  Char  = Character.Character;
  float Scale = Character.Scale;

  if (Scale <= 0.0f)
    Scale = 1.0f;

  const uint8 *Glyph = Font[(unsigned char)Char];

  int PosX = Character.Position.x;
  int PosY = Character.Position.y;

  for (int y = 0; y < FONT_HEIGHT; y++)
  {
    uint8 Row = Glyph[y];

    for (int x = 0; x < FONT_WIDTH; x++)
    {
      if (!(Row & (1 << x)))
        continue;

      int StartX = (int)(x * Scale);
      int EndX   = (int)((x + 1) * Scale);

      int StartY = (int)(y * Scale);
      int EndY   = (int)((y + 1) * Scale);

      for (int py = StartY; py < EndY; py++)
      {
        for (int px = StartX; px < EndX; px++)
          put_pixel(Buffer, PosX + px, PosY + py, Character.Color);
      }
    }
  }
}

void draw_string(framebuffer Buffer, string String)
{
  if (String.Color.a == 0)
    return;

  uVector     Position    = String.Position;
  float       Scale       = String.Scale;
  const char *cString     = String.String;
  color       Color       = String.Color;
  int         LineSpacing = String.LineSpacing;
  int         TabWidth    = String.TabWidth;

  int x = 0;
  int y = 0;

  int LineHeight = (int)(FONT_HEIGHT * Scale) + LineSpacing;

  while (*cString)
  {
    character Character;

    Character.Position = (uVector){ Position.x + (int)(x * FONT_WIDTH * Scale),
                                    Position.y + (int)(y * LineHeight) };

    Character.Scale     = Scale;
    Character.Character = *cString;
    Character.Color     = Color;

    if (*cString != '\n' && *cString != '\t')
      draw_char(Buffer, Character);

    if (*cString == '\n')
    {
      x = 0;
      y++;
    }
    else if (*cString == '\t')
    {
      x += TabWidth;
    }
    else
    {
      x++;
    }

    cString++;
  }
}

void draw_raw(framebuffer Buffer, raw_pixels Raw)
{
  const unsigned int SourceWidth  = Raw.SourceSize.x;
  const unsigned int SourceHeight = Raw.SourceSize.y;

  const unsigned int DestWidth  = Raw.Size.x;
  const unsigned int DestHeight = Raw.Size.y;

  if (SourceWidth == 0 || SourceHeight == 0 || DestWidth == 0 ||
      DestHeight == 0)
    return;

  for (unsigned int y = 0; y < DestHeight; y++)
  {
    unsigned int SourceY = (y * SourceHeight) / DestHeight;

    int DstY = (int)Raw.Position.y + (int)y;

    if (DstY < 0 || (unsigned int)DstY >= Buffer.Height)
      continue;

    for (unsigned int x = 0; x < DestWidth; x++)
    {
      unsigned int SourceX = (x * SourceWidth) / DestWidth;

      int DstX = (int)Raw.Position.x + (int)x;

      if (DstX < 0 || (unsigned int)DstX >= Buffer.Width)
        continue;

      color Pixel = Raw.Pixels[SourceY * SourceWidth + SourceX];

      if (Raw.Channels == 3)
        Pixel.a = 255;

      put_pixel(Buffer, DstX, DstY, Pixel);
    }
  }
}

int get_string_width(string String)
{
  int x    = 0;
  int newX = 0;

  while (*String.String != '\0')
  {
    if (*String.String == '\n')
    {
      if (newX > x)
        x = newX;

      newX = 0;
    }
    else if (*String.String == '\t')
    {
      newX += String.TabWidth;
    }
    else
    {
      newX++;
    }

    String.String++;
  }

  if (newX > x)
    x = newX;

  return (int)(x * FONT_WIDTH * String.Scale);
}

int get_string_height(string String)
{
  int y = 1;

  while (*String.String != '\0')
  {
    if (*String.String == '\n')
      y++;

    String.String++;
  }

  return (int)(y * FONT_HEIGHT * String.Scale);
}

uVector get_string_size(string String)
{
  return (uVector){ get_string_width(String), get_string_height(String) };
}

static framebuffer *RealBuffer;
static framebuffer  BackBuffer;

void graphics_init(framebuffer *RealFramebuffer, unsigned int BackbufferAddress)
{
  RealBuffer = RealFramebuffer;

  BackBuffer         = *RealFramebuffer;
  BackBuffer.Address = BackbufferAddress;
}

framebuffer *get_backbuffer(void)
{
  return &BackBuffer;
}

void end_drawing(void)
{
  unsigned int Count = (BackBuffer.Pitch * BackBuffer.Height) / 4;

  memcpy32((void *)RealBuffer->Address, (void *)BackBuffer.Address, Count);
}
