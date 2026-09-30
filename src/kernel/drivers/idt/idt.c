#include "idt.h"

#include "drivers/pic/pic.h"

#define IDT_ENTRIES 256

static idt_entry IDT[IDT_ENTRIES];
static idt_ptr   IDTP;

extern void irq0_stub(void);
extern void irq1_stub(void);
extern void irq12_stub(void);

void idt_set_gate(uint8 Number, uint32 Handler)
{
  IDT[Number].OffsetLow      = Handler & 0xFFFF;
  IDT[Number].Selector       = 0x08;
  IDT[Number].Zero           = 0;
  IDT[Number].TypeAttributes = 0x8E;
  IDT[Number].OffsetHigh     = (Handler >> 16) & 0xFFFF;
}

void idt_init(void)
{
  for (int i = 0; i < IDT_ENTRIES; i++)
  {
    IDT[i].OffsetLow      = 0;
    IDT[i].Selector       = 0;
    IDT[i].Zero           = 0;
    IDT[i].TypeAttributes = 0;
    IDT[i].OffsetHigh     = 0;
  }

  IDTP.Limit = sizeof(IDT) - 1;
  IDTP.Base  = (uint32)IDT;

  __asm__ volatile("lidt %0" : : "m"(IDTP));

  idt_set_gate(0x20, (uint32)irq0_stub);
  idt_set_gate(0x21, (uint32)irq1_stub);
  idt_set_gate(0x2C, (uint32)irq12_stub);

  pic_remap();
}
