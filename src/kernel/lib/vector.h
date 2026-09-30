#pragma once

#include <stddef.h>

#define VECTOR_INITIAL_CAPACITY 8

typedef struct
{
  void  *Content;
  size_t Length;
  size_t Capacity;

  size_t SizeOfItems;
} vector;

void vector_init(vector *Vector);

void vector_insert_impl(vector *Vector, size_t Position, void *Item);
void vector_pushback_impl(vector *Vector, void *Item);

void vector_remove(vector *Remove, size_t Position);
void vector_popback(vector *Vector);
