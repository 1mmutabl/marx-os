#include "drivers/pic/pic.h"

void mouse_handler()
{
  pic_send_eoi(12);
}
