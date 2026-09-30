bits 32

global kernel_start
global irq0_stub
global irq1_stub
global irq12_stub

global g_TotalRamKB
global g_HeapStart
global g_HeapSize
global g_StackSize

extern kmain
extern pit_handler
extern keyboard_handler
extern mouse_handler

MMAP_TOTAL_KB_ADDR equ 0x9604

FALLBACK_TOTAL_KB equ 16384

MIN_VIABLE_RAM_BYTES equ 0xD00000

HEAP_START      equ 0x800000
HEAP_MIN_SIZE   equ 0x400000
HEAP_MAX_SIZE   equ 0x4000000
HEAP_DIVISOR    equ 4

STACK_MIN_SIZE  equ 0x40000
STACK_MAX_SIZE  equ 0x400000
STACK_DIVISOR   equ 16

kernel_start:
  mov [g_FbAddress], eax
  mov [g_FbPitch], ebx
  mov [g_FbWidth], ecx
  mov [g_FbHeight], edx

  mov eax, cr0
  and eax, ~(1 << 2)
  and eax, ~(1 << 3)
  or  eax,  (1 << 1)
  mov cr0, eax

  fninit

  mov eax, cr4
  or  eax, (1 << 9)
  or  eax, (1 << 10)
  mov cr4, eax

  mov eax, [MMAP_TOTAL_KB_ADDR]
  test eax, eax
  jnz .have_ram
  mov eax, FALLBACK_TOTAL_KB
  .have_ram:
  mov [g_TotalRamKB], eax

  mov ebx, eax
  shl ebx, 10

  cmp ebx, MIN_VIABLE_RAM_BYTES
  jae .compute_dynamic

  mov dword [g_HeapSize], HEAP_MIN_SIZE
  mov dword [g_StackSize], STACK_MIN_SIZE
  jmp .layout_done

  .compute_dynamic:
    mov eax, ebx
    xor edx, edx
    mov ecx, HEAP_DIVISOR
    div ecx
    cmp eax, HEAP_MIN_SIZE
    jae .heap_above_min
    mov eax, HEAP_MIN_SIZE
    .heap_above_min:
    cmp eax, HEAP_MAX_SIZE
    jbe .heap_below_max
    mov eax, HEAP_MAX_SIZE
    .heap_below_max:
    mov [g_HeapSize], eax

    mov eax, ebx
    xor edx, edx
    mov ecx, STACK_DIVISOR
    div ecx
    cmp eax, STACK_MIN_SIZE
    jae .stack_above_min
    mov eax, STACK_MIN_SIZE
    .stack_above_min:
    cmp eax, STACK_MAX_SIZE
    jbe .stack_below_max
    mov eax, STACK_MAX_SIZE
    .stack_below_max:
    mov [g_StackSize], eax

    mov eax, HEAP_START
    add eax, [g_HeapSize]
    add eax, [g_StackSize]
    add eax, 0x100000
    cmp eax, ebx
    jbe .layout_done

    mov dword [g_HeapSize], HEAP_MIN_SIZE
    mov dword [g_StackSize], STACK_MIN_SIZE

  .layout_done:
  mov dword [g_HeapStart], HEAP_START

  mov eax, HEAP_START
  add eax, [g_HeapSize]
  add eax, [g_StackSize]
  mov esp, eax
  mov ebp, esp

  push dword [g_FbHeight]
  push dword [g_FbWidth]
  push dword [g_FbPitch]
  push dword [g_FbAddress]
  call kmain

  .hang:
    cli
    hlt
    jmp .hang

irq0_stub:
  pusha
  call pit_handler
  popa
  iretd

irq1_stub:
  pusha
  call keyboard_handler
  popa
  iretd

irq12_stub:
  pusha
  call mouse_handler
  popa
  iretd

section .bss
align 4
g_FbAddress:  resd 1
g_FbPitch:    resd 1
g_FbWidth:    resd 1
g_FbHeight:   resd 1

g_TotalRamKB: resd 1
g_HeapStart:  resd 1
g_HeapSize:   resd 1
g_StackSize:  resd 1
