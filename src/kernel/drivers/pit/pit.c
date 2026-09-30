#include "pit.h"

#include "drivers/io/io.h"
#include "drivers/pic/pic.h"

extern void keyboard_update(void);

static volatile uint32 Ticks = 0;

void pit_init(uint32 Frequency)
{
  uint32 Divisor = PIT_FREQUENCY / Frequency;

  port_outb(PIT_COMMAND, 0x36);
  port_outb(PIT_CHANNEL0, Divisor & 0xFF);
  port_outb(PIT_CHANNEL0, (Divisor >> 8) & 0xFF);
}

uint32 pit_get_ticks(void)
{
  return Ticks;
}

void pit_handler(void)
{
  keyboard_update();

  Ticks++;
  pic_send_eoi(0);
}

void pit_sleep(uint32 Milliseconds)
{
  uint32 Start = Ticks;

  while ((Ticks - Start) < Milliseconds)
    __asm__ volatile("hlt");
}
