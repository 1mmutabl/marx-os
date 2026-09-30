#include <drivers/io/io.h>
#include <drivers/keyboard/keyboard.h>
#include <drivers/pic/pic.h>
#include <string.h>

#define KEYBOARD_DATA_PORT 0x60

static uint64   LastID    = 1;
static kb_event LastEvent = { .Scancode = 0x00,
                              .Extended = 0x00,
                              .State    = KB_NONE,
                              .Ascii    = '\0',
                              .Key      = KEY_NONE,
                              .ID       = 1 };

// clang-format off

static const char Scancodes[0x59] = {
  [0x01] = '\e',
  [0x02] = '1',
  [0x03] = '2',
  [0x04] = '3',
  [0x05] = '4',
  [0x06] = '5',
  [0x07] = '6',
  [0x08] = '7',
  [0x09] = '8',
  [0x0A] = '9',
  [0x0B] = '0',
  [0x0C] = '-',
  [0x0D] = '=',
  [0x0E] = '\b',
  [0x0F] = '\t',

  [0x10] = 'q',
  [0x11] = 'w',
  [0x12] = 'e',
  [0x13] = 'r',
  [0x14] = 't',
  [0x15] = 'y',
  [0x16] = 'u',
  [0x17] = 'i',
  [0x18] = 'o',
  [0x19] = 'p',
  [0x1A] = '[',
  [0x1B] = ']',
  [0x1C] = '\n',

  [0x1D] = 0,       // Left Ctrl

  [0x1E] = 'a',
  [0x1F] = 's',
  [0x20] = 'd',
  [0x21] = 'f',
  [0x22] = 'g',
  [0x23] = 'h',
  [0x24] = 'j',
  [0x25] = 'k',
  [0x26] = 'l',
  [0x27] = ';',
  [0x28] = '\'',
  [0x29] = '`',

  [0x2A] = 0,       // Left Shift

  [0x2B] = '\\',

  [0x2C] = 'z',
  [0x2D] = 'x',
  [0x2E] = 'c',
  [0x2F] = 'v',
  [0x30] = 'b',
  [0x31] = 'n',
  [0x32] = 'm',
  [0x33] = ',',
  [0x34] = '.',
  [0x35] = '/',

  [0x36] = 0,       // Right Shift
  [0x37] = '*',     // Keypad *
  [0x38] = 0,       // Left Alt
  [0x39] = ' ',

  [0x3A] = 0,       // Caps Lock

  [0x3B] = 0,       // F1
  [0x3C] = 0,       // F2
  [0x3D] = 0,       // F3
  [0x3E] = 0,       // F4
  [0x3F] = 0,       // F5
  [0x40] = 0,       // F6
  [0x41] = 0,       // F7
  [0x42] = 0,       // F8
  [0x43] = 0,       // F9
  [0x44] = 0,       // F10

  [0x45] = 0,       // Num Lock
  [0x46] = 0,       // Scroll Lock

  [0x47] = '7',     // Keypad 7 / Home
  [0x48] = '8',     // Keypad 8 / Up
  [0x49] = '9',     // Keypad 9 / Page Up
  [0x4A] = '-',

  [0x4B] = '4',     // Keypad 4 / Left
  [0x4C] = '5',     // Keypad 5
  [0x4D] = '6',     // Keypad 6 / Right
  [0x4E] = '+',

  [0x4F] = '1',     // Keypad 1 / End
  [0x50] = '2',     // Keypad 2 / Down
  [0x51] = '3',     // Keypad 3 / Page Down
  [0x52] = '0',     // Keypad 0 / Insert
  [0x53] = '.',     // Keypad . / Delete

  [0x54] = 0,
  [0x55] = 0,
  [0x56] = '<',     // Non-US backslash / pipe key
  [0x57] = 0,       // F11
  [0x58] = 0,       // F12
};

static const char Upper[] = {
  ['q'] = 'Q',
  ['w'] = 'W',
  ['e'] = 'E',
  ['r'] = 'R',
  ['t'] = 'T',
  ['y'] = 'Y',
  ['u'] = 'U',
  ['i'] = 'I',
  ['o'] = 'O',
  ['p'] = 'P',

  ['a'] = 'A',
  ['s'] = 'S',
  ['d'] = 'D',
  ['f'] = 'F',
  ['g'] = 'G',
  ['h'] = 'H',
  ['j'] = 'J',
  ['k'] = 'K',
  ['l'] = 'L',

  ['z'] = 'Z',
  ['x'] = 'X',
  ['c'] = 'C',
  ['v'] = 'V',
  ['b'] = 'B',
  ['n'] = 'N',
  ['m'] = 'M',

  ['1'] = '!',
  ['2'] = '@',
  ['3'] = '#',
  ['4'] = '$',
  ['5'] = '%',
  ['6'] = '^',
  ['7'] = '&',
  ['8'] = '*',
  ['9'] = '(',
  ['0'] = ')',

  ['-'] = '_',
  ['='] = '+',

  ['['] = '{',
  [']'] = '}',
  ['\\'] = '|',

  [';'] = ':',
  ['\''] = '"',
  ['`'] = '~',

  [','] = '<',
  ['.'] = '>',
  ['/'] = '?',

  ['<'] = '>'
};

static const kb_key ScancodeKeys[0x59] = {
  [0x01] = KEY_ESCAPE,

  [0x02] = KEY_1,
  [0x03] = KEY_2,
  [0x04] = KEY_3,
  [0x05] = KEY_4,
  [0x06] = KEY_5,
  [0x07] = KEY_6,
  [0x08] = KEY_7,
  [0x09] = KEY_8,
  [0x0A] = KEY_9,
  [0x0B] = KEY_0,
  [0x0C] = KEY_MINUS,
  [0x0D] = KEY_EQUALS,
  [0x0E] = KEY_BACKSPACE,
  [0x0F] = KEY_TAB,

  [0x10] = KEY_Q,
  [0x11] = KEY_W,
  [0x12] = KEY_E,
  [0x13] = KEY_R,
  [0x14] = KEY_T,
  [0x15] = KEY_Y,
  [0x16] = KEY_U,
  [0x17] = KEY_I,
  [0x18] = KEY_O,
  [0x19] = KEY_P,
  [0x1A] = KEY_LEFT_BRACKET,
  [0x1B] = KEY_RIGHT_BRACKET,
  [0x1C] = KEY_ENTER,

  [0x1D] = KEY_LEFT_CTRL,

  [0x1E] = KEY_A,
  [0x1F] = KEY_S,
  [0x20] = KEY_D,
  [0x21] = KEY_F,
  [0x22] = KEY_G,
  [0x23] = KEY_H,
  [0x24] = KEY_J,
  [0x25] = KEY_K,
  [0x26] = KEY_L,
  [0x27] = KEY_SEMICOLON,
  [0x28] = KEY_APOSTROPHE,
  [0x29] = KEY_GRAVE,

  [0x2A] = KEY_LEFT_SHIFT,
  [0x2B] = KEY_BACKSLASH,

  [0x2C] = KEY_Z,
  [0x2D] = KEY_X,
  [0x2E] = KEY_C,
  [0x2F] = KEY_V,
  [0x30] = KEY_B,
  [0x31] = KEY_N,
  [0x32] = KEY_M,
  [0x33] = KEY_COMMA,
  [0x34] = KEY_PERIOD,
  [0x35] = KEY_SLASH,

  [0x36] = KEY_RIGHT_SHIFT,
  [0x37] = KEY_NUMPAD_MULTIPLY,
  [0x38] = KEY_LEFT_ALT,
  [0x39] = KEY_SPACE,

  [0x3A] = KEY_CAPS_LOCK,

  [0x3B] = KEY_F1,
  [0x3C] = KEY_F2,
  [0x3D] = KEY_F3,
  [0x3E] = KEY_F4,
  [0x3F] = KEY_F5,
  [0x40] = KEY_F6,
  [0x41] = KEY_F7,
  [0x42] = KEY_F8,
  [0x43] = KEY_F9,
  [0x44] = KEY_F10,

  [0x45] = KEY_NUM_LOCK,
  [0x46] = KEY_SCROLL_LOCK,

  [0x47] = KEY_NUMPAD_7,
  [0x48] = KEY_NUMPAD_8,
  [0x49] = KEY_NUMPAD_9,
  [0x4A] = KEY_NUMPAD_MINUS,

  [0x4B] = KEY_NUMPAD_4,
  [0x4C] = KEY_NUMPAD_5,
  [0x4D] = KEY_NUMPAD_6,
  [0x4E] = KEY_NUMPAD_PLUS,

  [0x4F] = KEY_NUMPAD_1,
  [0x50] = KEY_NUMPAD_2,
  [0x51] = KEY_NUMPAD_3,
  [0x52] = KEY_NUMPAD_0,
  [0x53] = KEY_NUMPAD_DECIMAL,

  [0x56] = KEY_NONE, // Non-US backslash (not in kb_key)

  [0x57] = KEY_F11,
  [0x58] = KEY_F12,
};

static const kb_key ExtendedScancodeKeys[0x100] = {
  [0x1C] = KEY_NUMPAD_ENTER,
  [0x1D] = KEY_RIGHT_CTRL,

  [0x35] = KEY_NUMPAD_DIVIDE,
  [0x38] = KEY_RIGHT_ALT,

  [0x47] = KEY_HOME,
  [0x48] = KEY_UP,
  [0x49] = KEY_PAGE_UP,

  [0x4B] = KEY_LEFT,
  [0x4D] = KEY_RIGHT,

  [0x4F] = KEY_END,
  [0x50] = KEY_DOWN,
  [0x51] = KEY_PAGE_DOWN,

  [0x52] = KEY_INSERT,
  [0x53] = KEY_DELETE,
};

static kb_state HeldKeys[104];

// clang-format on

kb_key scancode_to_key(uint8 Scancode, bool Extended)
{
  if (Scancode >= 0x59)
    return KEY_NONE;

  return Extended ? ExtendedScancodeKeys[Scancode] : ScancodeKeys[Scancode];
}

void keyboard_update(void)
{
}

bool keyboard_held(kb_key Key)
{
  return HeldKeys[Key] == KB_PRESS;
}

void keyboard_get(kb_event *Event)
{
  *Event = LastEvent;
}

void keyboard_handler(void)
{
  uint8 Scancode = port_inb(0x60);

  bool Released = false;
  bool Extended = false;

  if (Scancode & 0x80)
  {
    Scancode &= 0x7F;
    Released = true;
  }

  kb_state State = Released ? KB_RELEASE : KB_PRESS;

  if (Scancode == 0xE0)
  {
    Scancode = port_inb(0x60);
    Extended = true;
  }

  kb_key Key = scancode_to_key(Scancode, Extended);

  HeldKeys[Key] = State;

  char Ascii = '\0';
  Ascii      = Scancodes[Scancode];

  bool Shifted =
      keyboard_held(KEY_LEFT_SHIFT) || keyboard_held(KEY_RIGHT_SHIFT);
  if (Shifted)
    Ascii = Upper[Ascii];

  kb_event Event;

  Event.Scancode = Scancode;
  Event.Ascii    = Ascii;
  Event.Key      = Key;
  Event.State    = State;
  Event.ID       = LastID++;

  LastEvent = Event;
  pic_send_eoi(1);
}
