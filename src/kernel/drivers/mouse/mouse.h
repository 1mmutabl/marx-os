#pragma once

#include <graphics/graphics.h>
#include <types.h>
#include <vector.h>

typedef struct
{
  bool    Buttons[3];
  uVector Position;
  uint64  ID;
} __attribute__((packed)) ms_event;

void mouse_init(framebuffer Buffer);
void mouse_get(ms_event *Event);
