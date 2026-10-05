#pragma once

#include <types.h>
#include <vector.h>

#include "graphics/graphics.h"

typedef struct
{
  uVector     Position;
  uVector     Size;
  const char *Title;
} window;

void de_init(framebuffer Buffer);
void de_draw();
