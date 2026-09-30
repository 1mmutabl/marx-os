#include <drivers/io/io.h>
#include <drivers/keyboard/keyboard.h>
#include <drivers/pic/pic.h>
#include <fs/fs.h>
#include <graphics/graphics.h>
#include <lib/heap.h>
#include <types.h>

#include "drivers/idt/idt.h"
#include "drivers/pit/pit.h"

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

void kmain(framebuffer Framebuffer)
{
  mtrr_set_write_combining(Framebuffer.Address,
                           Framebuffer.Pitch * Framebuffer.Height);

  graphics_init(&Framebuffer, BACKBUFFER_ADDRESS);
  framebuffer *Buffer = get_backbuffer();

  fs_init();

  heap_init(g_HeapStart, g_HeapSize);
  idt_init();
  pit_init(1000);

  __asm__ volatile("sti");

  char Buff[1024];
  size i = 0;

  uint64 LastID = 0;

  while (1)
  {
    kb_event Event;
    file_read("/system/input/keyboard.sys", &Event, sizeof(kb_event));

    if (Event.ID > LastID)
    {
      LastID = Event.ID;

      if (Event.State == KB_PRESS)
      {
        if (Event.Key == KEY_BACKSPACE)
          Buff[--i] = '\0';
        else if (Event.Ascii != '\0')
          Buff[i++] = Event.Ascii;
      }
    }

    string String;

    String.String      = Buff;
    String.Position    = (uVector){ 10, 10 };
    String.Color       = COLOR_LIGHT_GRAY;
    String.Scale       = 2.f;
    String.LineSpacing = 2;
    String.TabWidth    = 8;

    clear_screen(Buffer, COLOR_BLACK);
    draw_string(Buffer, &String);
    end_drawing();
  }
}
