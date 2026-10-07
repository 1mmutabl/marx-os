#include <desktop/desktop.h>
#include <fs/fs.h>
#include <types.h>

#define BACKGROUND_PATH "/system/background/bg-1.jpg"

framebuffer Buffer;
static int  taskbar_Height = 32;

void de_init(framebuffer Framebuffer)
{
  Buffer = Framebuffer;

  if (!file_exists(BACKGROUND_PATH))
    return;

  uint bg_Size = file_size(BACKGROUND_PATH);
  char bg_Read[bg_Size];
  file_read(BACKGROUND_PATH, bg_Read, bg_Size);
}

void de_draw()
{
  rect Taskbar;

  Taskbar.Position   = (uVector){ 10, Buffer.Height - taskbar_Height * 2 };
  Taskbar.Size       = (uVector){ Buffer.Width - 20, taskbar_Height };
  Taskbar.Color      = (color){ 50, 50, 50, 255 };
  Taskbar.shadow_Use = true;

  Taskbar.Shadow.BlurRadius = 10;
  Taskbar.Shadow.Color      = COLOR_BLACK;
  Taskbar.Shadow.Offset     = (uVector){ 0, 0 };
  Taskbar.Shadow.Spread     = 0;
  Taskbar.Shadow.Opacity    = 0.5 * 255;

  draw_rect(Buffer, Taskbar);
}
