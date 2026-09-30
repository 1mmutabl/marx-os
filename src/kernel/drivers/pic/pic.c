#include "pic.h"

#include "drivers/io/io.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21

#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

#define PIC_EOI 0x20

#define PIC1_OFFSET 0x20
#define PIC2_OFFSET 0x28

void pic_remap(void)
{
  port_outb(PIC1_COMMAND, 0x11);
  port_outb(PIC2_COMMAND, 0x11);

  port_outb(PIC1_DATA, PIC1_OFFSET);
  port_outb(PIC2_DATA, PIC2_OFFSET);

  port_outb(PIC1_DATA, 0x04);

  port_outb(PIC2_DATA, 0x02);

  port_outb(PIC1_DATA, 0x01);
  port_outb(PIC2_DATA, 0x01);

  port_outb(PIC1_DATA, 0xF8);

  port_outb(PIC2_DATA, 0xEF);
}

void pic_send_eoi(uint8 irq)
{
  if (irq >= 8)
    port_outb(PIC2_COMMAND, PIC_EOI);

  port_outb(PIC1_COMMAND, PIC_EOI);
}
