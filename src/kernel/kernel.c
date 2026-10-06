#include <desktop/desktop.h>
#include <drivers/idt/idt.h>
#include <drivers/io/io.h>
#include <drivers/keyboard/keyboard.h>
#include <drivers/mouse/mouse.h>
#include <drivers/pic/pic.h>
#include <drivers/pit/pit.h>
#include <fs/fs.h>
#include <graphics/graphics.h>
#include <lib/heap.h>
#include <lib/tween.h>
#include <stdio.h>
#include <types.h>

int int_to_string(int value, char *buffer, int size)
{
  unsigned int magnitude;
  int          digits = 1;
  int          required;
  int          i;
  int          left;
  int          right;
  char         temp;

  if (buffer == 0 || size <= 0)
    return 0;

  if (value < 0)
    magnitude = 0u - (unsigned int)value;
  else
    magnitude = (unsigned int)value;

  {
    unsigned int copy = magnitude;

    while (copy >= 10)
    {
      copy /= 10;
      digits++;
    }
  }

  required = digits + (value < 0 ? 1 : 0) + 1; /* sign + '\0' */

  if (size < required)
  {
    buffer[0] = '\0';
    return 0;
  }

  i = 0;

  if (value < 0)
    buffer[i++] = '-';

  left = i;

  do
  {
    buffer[i++] = (char)('0' + magnitude % 10);
    magnitude /= 10;
  } while (magnitude != 0);

  buffer[i] = '\0';

  right = i - 1;

  while (left < right)
  {
    temp          = buffer[left];
    buffer[left]  = buffer[right];
    buffer[right] = temp;

    left++;
    right--;
  }

  return 1;
}

#define BACKBUFFER_ADDRESS 0x400000

#define CPUID_FEAT_EDX_MTRR (1 << 12)
#define IA32_MTRRCAP 0xFE
#define IA32_MTRR_DEF_TYPE 0x2FF
#define IA32_MTRR_PHYSBASE0 0x200
#define IA32_MTRR_PHYSMASK0 0x201
#define MTRR_TYPE_WC 0x01

extern uint32 g_TotalRamKB;
extern uint32 g_HeapStart;
extern uint32 g_HeapSize;
extern uint32 g_StackSize;

static inline void cpuid(uint32 Leaf, uint32 *Eax, uint32 *Ebx, uint32 *Ecx,
                         uint32 *Edx)
{
  __asm__ volatile("cpuid"
                   : "=a"(*Eax), "=b"(*Ebx), "=c"(*Ecx), "=d"(*Edx)
                   : "a"(Leaf));
}

static inline void rdmsr(uint32 Msr, uint32 *Lo, uint32 *Hi)
{
  __asm__ volatile("rdmsr" : "=a"(*Lo), "=d"(*Hi) : "c"(Msr));
}

static inline void wrmsr(uint32 Msr, uint32 Lo, uint32 Hi)
{
  __asm__ volatile("wrmsr" : : "a"(Lo), "d"(Hi), "c"(Msr));
}

static uint32 next_pow2(uint32 Value)
{
  Value--;
  Value |= Value >> 1;
  Value |= Value >> 2;
  Value |= Value >> 4;
  Value |= Value >> 8;
  Value |= Value >> 16;
  Value++;
  return Value;
}

static void mtrr_set_write_combining(uint32 PhysBase, uint32 MinSize)
{
  uint32 Eax, Ebx, Ecx, Edx;

  cpuid(1, &Eax, &Ebx, &Ecx, &Edx);

  if (!(Edx & CPUID_FEAT_EDX_MTRR))
    return;

  uint32 CapLo, CapHi;
  rdmsr(IA32_MTRRCAP, &CapLo, &CapHi);

  int VarCount    = CapLo & 0xFF;
  int WcSupported = (CapLo >> 10) & 1;

  if (!WcSupported || VarCount == 0)
    return;

  uint32 Size = next_pow2(MinSize);

  while (PhysBase & (Size - 1))
    Size <<= 1;

  int Slot = -1;

  for (int i = 0; i < VarCount; i++)
  {
    uint32 MaskLo, MaskHi;

    rdmsr(IA32_MTRR_PHYSMASK0 + i * 2, &MaskLo, &MaskHi);

    if (!(MaskLo & (1 << 11)))
    {
      Slot = i;
      break;
    }
  }

  if (Slot < 0)
    return;

  uint32 MaxExtLeaf, Unused1, Unused2, Unused3;

  cpuid(0x80000000, &MaxExtLeaf, &Unused1, &Unused2, &Unused3);

  int PhysBits = 36;

  if (MaxExtLeaf >= 0x80000008)
  {
    uint32 A, B, C, D;

    cpuid(0x80000008, &A, &B, &C, &D);

    PhysBits = A & 0xFF;
  }

  uint32 MaskHi = (PhysBits > 32) ? ((1u << (PhysBits - 32)) - 1) : 0;

  uint32 MaskLo = (~(Size - 1)) | (1 << 11);

  wrmsr(IA32_MTRR_PHYSBASE0 + Slot * 2, (PhysBase & 0xFFFFF000) | MTRR_TYPE_WC,
        0);

  wrmsr(IA32_MTRR_PHYSMASK0 + Slot * 2, MaskLo, MaskHi);

  uint32 DefLo, DefHi;

  rdmsr(IA32_MTRR_DEF_TYPE, &DefLo, &DefHi);

  DefLo |= (1 << 11);

  wrmsr(IA32_MTRR_DEF_TYPE, DefLo, DefHi);
}

void init(framebuffer *Buffer)
{
  mtrr_set_write_combining(Buffer->Address, Buffer->Pitch * Buffer->Height);
  graphics_init(Buffer, BACKBUFFER_ADDRESS);

  fs_init();

  heap_init(g_HeapStart, g_HeapSize);
  idt_init();
  pit_init(1000);
  mouse_init(*Buffer);

  __asm__ volatile("sti");
}

void kmain(framebuffer Framebuffer)
{
  init(&Framebuffer);
  framebuffer Buffer = *get_backbuffer();

  pit_sleep(1000);

  tween WelcomeTween;
  tween_init(&WelcomeTween, 0.f, 255.f, 3.f, TWEEN_EASE_OUT_QUINT);
  tween_start(&WelcomeTween);

  uint32 Before = pit_get_ticks();
  while (!tween_is_finished(&WelcomeTween))
  {
    uint32 Now   = pit_get_ticks();
    float  Delta = (Now - Before) / 1000.f;
    Before       = Now;

    tween_update(&WelcomeTween, Delta);
    float Value = tween_get_value(&WelcomeTween);

    string Welcome;

    Welcome.String = "Welcome to Marx-OS";
    Welcome.Scale  = 3.f;
    Welcome.Color  = (color){ 255, 255, 255, Value };

    uVector WelcomeSize = get_string_size(Welcome);

    Welcome.Position = (uVector){ Buffer.Width / 2.f - WelcomeSize.x / 2.f,
                                  Buffer.Height / 2.f - WelcomeSize.y / 2.f };

    clear_screen(Buffer, COLOR_BLACK);
    draw_string(Buffer, Welcome);
    end_drawing();
  }

  tween bgTween;
  tween_init(&bgTween, 0.f, 255.f, 3.f, TWEEN_EASE_OUT_QUINT);
  tween_start(&bgTween);

  Before = pit_get_ticks();
  while (!tween_is_finished(&bgTween))
  {
    uint32 Now   = pit_get_ticks();
    float  Delta = (Now - Before) / 1000.f;
    Before       = Now;

    tween_update(&bgTween, Delta);
    float Value = tween_get_value(&bgTween);

    string Welcome;

    Welcome.String = "Welcome to Marx-OS";
    Welcome.Scale  = 3.f;
    Welcome.Color  = COLOR_WHITE;

    uVector WelcomeSize = get_string_size(Welcome);

    Welcome.Position = (uVector){ Buffer.Width / 2.f - WelcomeSize.x / 2.f,
                                  Buffer.Height / 2.f - WelcomeSize.y / 2.f };

    clear_screen(Buffer, (color){ Value, Value, Value, 255 });
    draw_string(Buffer, Welcome);
    end_drawing();
  }

  string Wait;

  Wait.String = "Wait while Marx-OS is\ninitializing other stuff.";
  Wait.Scale  = 2.f;
  Wait.Color  = (color){ 32, 32, 32, 0.9 * 255 };

  uVector WaitSize = get_string_size(Wait);
  Wait.Position    = (uVector){ Buffer.Width / 2.f - WaitSize.x / 2.f,
                                Buffer.Height / 2.f - WaitSize.y / 2.f };

  Wait.LineSpacing = 5;

  clear_screen(Buffer, COLOR_WHITE);
  draw_string(Buffer, Wait);
  end_drawing();

  de_init(Buffer);

  clear_screen(Buffer, COLOR_BLACK);
  // de_draw();
  end_drawing();

  for (;;)
  {
    ms_event Event;
    mouse_get(&Event);

    rect Cursor;

    Cursor.Position = Event.Position;
    Cursor.Size     = (uVector){ 10, 20 };
    Cursor.Color    = COLOR_LIGHT_GRAY;

    char Buff[1024];
    snprintf(Buff, sizeof(Buff),
             "Left: %d\nRight: %d\nMiddle: %d\n\ndid you know jinxy is gay?",
             Event.Buttons[0], Event.Buttons[1], Event.Buttons[2]);

    string String;

    String.String   = Buff;
    String.Position = (uVector){ 10, 250 };
    String.Color    = COLOR_LIGHT_GRAY;
    String.Scale    = 3.f;

    clear_screen(Buffer, COLOR_BLACK);
    draw_string(Buffer, String);
    draw_rect(Buffer, Cursor);
    end_drawing();
  }
}
