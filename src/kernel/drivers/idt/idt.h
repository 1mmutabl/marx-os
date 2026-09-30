#pragma once

#include <types.h>

typedef struct
{
  uint16 OffsetLow;
  uint16 Selector;
  uint8  Zero;
  uint8  TypeAttributes;
  uint16 OffsetHigh;
} __attribute__((packed)) idt_entry;

typedef struct
{
  uint16 Limit;
  uint32 Base;
} __attribute__((packed)) idt_ptr;

void idt_init(void);
void idt_set_gate(uint8 Number, uint32 Handler);
