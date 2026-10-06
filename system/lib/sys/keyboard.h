#ifndef SYS_KEYBOARD_H
#define SYS_KEYBOARD_H

#include <stdbool.h>
#include <types.h>

typedef enum
{
  KB_NONE,
  KB_RELEASE,
  KB_PRESS,
} kb_state;

// clang-format off

typedef enum
{
  KEY_NONE,

  KEY_ESCAPE,

  KEY_F1, KEY_F2, KEY_F3,
  KEY_F4, KEY_F5, KEY_F6,
  KEY_F7, KEY_F8, KEY_F9,
  KEY_F10, KEY_F11, KEY_F12,

  KEY_1, KEY_2, KEY_3,
  KEY_4, KEY_5, KEY_6,
  KEY_7, KEY_8, KEY_9,
  KEY_0,

  KEY_Q, KEY_W, KEY_E,
  KEY_R, KEY_T, KEY_Y,
  KEY_U, KEY_I, KEY_O,
  KEY_P,

  KEY_A, KEY_S, KEY_D,
  KEY_F, KEY_G, KEY_H,
  KEY_J, KEY_K, KEY_L,

  KEY_Z, KEY_X, KEY_C,
  KEY_V, KEY_B, KEY_N,
  KEY_M,

  KEY_MINUS, KEY_EQUALS, KEY_LEFT_BRACKET,
  KEY_RIGHT_BRACKET, KEY_BACKSLASH, KEY_SEMICOLON,
  KEY_APOSTROPHE, KEY_GRAVE, KEY_COMMA,
  KEY_PERIOD, KEY_SLASH,

  KEY_LEFT_CTRL, KEY_RIGHT_CTRL, KEY_LEFT_SHIFT,
  KEY_RIGHT_SHIFT, KEY_LEFT_ALT, KEY_RIGHT_ALT,
  KEY_CAPS_LOCK,

  KEY_TAB, KEY_ENTER, KEY_BACKSPACE,
  KEY_SPACE,

  KEY_NUM_LOCK, KEY_SCROLL_LOCK,

  KEY_UP, KEY_DOWN, KEY_LEFT,
  KEY_RIGHT, KEY_INSERT, KEY_DELETE,
  KEY_HOME, KEY_END, KEY_PAGE_UP,
  KEY_PAGE_DOWN,

  KEY_NUMPAD_0, KEY_NUMPAD_1, KEY_NUMPAD_2,
  KEY_NUMPAD_3, KEY_NUMPAD_4, KEY_NUMPAD_5,
  KEY_NUMPAD_6, KEY_NUMPAD_7, KEY_NUMPAD_8,
  KEY_NUMPAD_9, KEY_NUMPAD_DECIMAL, KEY_NUMPAD_PLUS,
  KEY_NUMPAD_MINUS, KEY_NUMPAD_MULTIPLY, KEY_NUMPAD_DIVIDE,
  KEY_NUMPAD_ENTER
} kb_key;

// clang-format on

typedef struct
{
  uint8    Scancode;
  bool     Extended;
  kb_state State;
  char     Ascii;
  kb_key   Key;
  uint64   ID;
} __attribute__((packed)) kb_event;

void keyboard_update(void);
bool keyboard_held(kb_key Key);
bool keyboard_capslock(void);
void keyboard_get(kb_event *Event);

#endif
