#include "heap.h"

#define HEAP_MAGIC 0xBEEF0001

static void *heap_memset(void *dest, int value, size_t n)
{
  uint8 *d = (uint8 *)dest;
  for (size_t i = 0; i < n; i++)
  {
    d[i] = (uint8)value;
  }
  return dest;
}

static void *heap_memcpy(void *dest, const void *src, size_t n)
{
  uint8       *d = (uint8 *)dest;
  const uint8 *s = (const uint8 *)src;
  for (size_t i = 0; i < n; i++)
  {
    d[i] = s[i];
  }
  return dest;
}

typedef struct block_header
{
  uint32               magic;
  size_t               size;
  uint8                free;
  struct block_header *next;
  struct block_header *prev;
} block_header_t;

#define HEADER_SIZE (sizeof(block_header_t))
#define ALIGNMENT 16u
#define ALIGN_UP(x) (((x) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define MIN_BLOCK_SIZE 32u

static block_header_t *heap_head  = NULL;
static uint32          heap_start = 0;
static uint32          heap_end   = 0;

void heap_init(uint32 start, uint32 size)
{
  heap_start = ALIGN_UP(start);
  heap_end   = start + size;

  heap_head        = (block_header_t *)heap_start;
  heap_head->magic = HEAP_MAGIC;
  heap_head->size  = heap_end - heap_start - HEADER_SIZE;
  heap_head->free  = 1;
  heap_head->next  = NULL;
  heap_head->prev  = NULL;
}

static block_header_t *find_free_block(size_t size)
{
  block_header_t *cur = heap_head;
  while (cur)
  {
    if (cur->free && cur->size >= size)
    {
      return cur;
    }
    cur = cur->next;
  }
  return NULL;
}

static void split_block(block_header_t *block, size_t size)
{
  size_t remaining = block->size - size;

  if (remaining < HEADER_SIZE + MIN_BLOCK_SIZE)
  {

    return;
  }

  block_header_t *new_block =
      (block_header_t *)((uint8 *)block + HEADER_SIZE + size);

  new_block->magic = HEAP_MAGIC;
  new_block->size  = remaining - HEADER_SIZE;
  new_block->free  = 1;
  new_block->next  = block->next;
  new_block->prev  = block;

  if (block->next)
  {
    block->next->prev = new_block;
  }
  block->next = new_block;
  block->size = size;
}

static void coalesce(block_header_t *block)
{
  if (!block)
    return;

  if (block->next && block->next->free)
  {
    block_header_t *next = block->next;
    block->size += HEADER_SIZE + next->size;
    block->next = next->next;
    if (next->next)
    {
      next->next->prev = block;
    }
  }

  if (block->prev && block->prev->free)
  {
    coalesce(block->prev);
  }
}

void *kmalloc(size_t size)
{
  if (size == 0 || heap_head == NULL)
  {
    return NULL;
  }

  size = ALIGN_UP(size);

  block_header_t *block = find_free_block(size);
  if (!block)
  {
    return NULL;
  }

  split_block(block, size);
  block->free = 0;

  return (void *)((uint8 *)block + HEADER_SIZE);
}

void *kcalloc(size_t num, size_t size)
{
  size_t total = num * size;
  void  *ptr   = kmalloc(total);
  if (ptr)
  {
    heap_memset(ptr, 0, total);
  }
  return ptr;
}

void kfree(void *ptr)
{
  if (!ptr)
    return;

  block_header_t *block = (block_header_t *)((uint8 *)ptr - HEADER_SIZE);

  if (block->magic != HEAP_MAGIC)
  {

    return;
  }

  block->free = 1;
  coalesce(block);
}

void *krealloc(void *ptr, size_t size)
{
  if (!ptr)
  {
    return kmalloc(size);
  }
  if (size == 0)
  {
    kfree(ptr);
    return NULL;
  }

  block_header_t *block = (block_header_t *)((uint8 *)ptr - HEADER_SIZE);

  if (block->magic != HEAP_MAGIC)
  {
    return NULL;
  }

  size_t aligned = ALIGN_UP(size);

  if (block->size >= aligned)
  {
    split_block(block, aligned);
    return ptr;
  }

  if (block->next && block->next->free &&
      block->size + HEADER_SIZE + block->next->size >= aligned)
  {
    coalesce(block);
    split_block(block, aligned);
    return ptr;
  }

  void *new_ptr = kmalloc(size);
  if (!new_ptr)
  {
    return NULL;
  }
  heap_memcpy(new_ptr, ptr, block->size);
  kfree(ptr);
  return new_ptr;
}

void heap_dump(void)
{
  block_header_t *cur = heap_head;
  while (cur)
  {

    cur = cur->next;
  }
}
