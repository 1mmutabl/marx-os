#include <drivers/io/io.h>
#include <drivers/mouse/mouse.h>
#include <drivers/pic/pic.h>
#include <vector.h>

#define MOUSE_DATA_PORT 0x60

static uint8 MousePacket[3];
static uint8 MousePacketIndex = 0;

static uint64 LastID = 1;

static ms_event LastEvent = { .Buttons = { false, false, false }, .ID = 1 };

static vector  CurrentPos = { 0, 0 };
static uVector Borders    = { 0 };

bool bit_set(uint8 Byte, uint8 Bit)
{
  return (Byte & (1 << Bit)) != 0;
}

static void mouse_wait_write(void)
{
  while (port_inb(0x64) & 0x02)
    ;
}

static void mouse_wait_read(void)
{
  while (!(port_inb(0x64) & 0x01))
    ;
}

void mouse_init(framebuffer Buffer)
{
  mouse_wait_write();
  port_outb(0x64, 0xA8);

  mouse_wait_write();
  port_outb(0x64, 0x20);

  mouse_wait_read();
  uint8 Status = port_inb(0x60);

  Status |= 0x02;

  mouse_wait_write();
  port_outb(0x64, 0x60);

  mouse_wait_write();
  port_outb(0x60, Status);

  mouse_wait_write();
  port_outb(0x64, 0xD4);

  mouse_wait_write();
  port_outb(0x60, 0xF4);

  mouse_wait_read();
  port_inb(0x60);

  Borders = (uVector){ Buffer.Width, Buffer.Height };
}

void mouse_get(ms_event *Event)
{
  *Event = LastEvent;
}

void mouse_handler(void)
{
  uint8 Data = port_inb(MOUSE_DATA_PORT);

  if (MousePacketIndex == 0)
  {
    if (!(Data & 0x08))
    {
      pic_send_eoi(12);
      return;
    }
  }

  MousePacket[MousePacketIndex++] = Data;

  if (MousePacketIndex < 3)
  {
    pic_send_eoi(12);
    return;
  }

  MousePacketIndex = 0;

  uint8 Flags = MousePacket[0];
  int8  x     = (int8)MousePacket[1];
  int8  y     = (int8)MousePacket[2];

  CurrentPos.x += x;
  CurrentPos.y -= y;

  if (CurrentPos.x < 0)
    CurrentPos.x = 0;
  if (CurrentPos.y < 0)
    CurrentPos.y = 0;

  if (CurrentPos.x >= Borders.x)
    CurrentPos.x = Borders.x - 1;
  if (CurrentPos.y >= Borders.y)
    CurrentPos.y = Borders.y - 1;

  LastEvent.Buttons[0] = bit_set(Flags, 0);
  LastEvent.Buttons[1] = bit_set(Flags, 1);
  LastEvent.Buttons[2] = bit_set(Flags, 2);

  LastEvent.Position = (uVector){ (uint)CurrentPos.x, (uint)CurrentPos.y };

  LastEvent.ID = LastID++;

  pic_send_eoi(12);
}
