#include "string.h"

#include <stdarg.h>

void *memset(void *dest, int value, uint32 count)
{
  uint8 *Bytes = (uint8 *)dest;

  uint32 Value = ((uint32)(uint8)value << 24) | ((uint32)(uint8)value << 16) |
                 ((uint32)(uint8)value << 8) | ((uint32)(uint8)value);

  uint32 *Words = (uint32 *)dest;

  uint32 WordCount = count / 4;

  for (uint32 i = 0; i < WordCount; i++)
    Words[i] = Value;

  for (uint32 i = WordCount * 4; i < count; i++)
    Bytes[i] = (uint8)value;

  return dest;
}

void *memcpy(void *dest, const void *src, uint32 count)
{
  uint32       *Dst = (uint32 *)dest;
  const uint32 *Src = (const uint32 *)src;

  uint32 Words = count / 4;

  for (uint32 i = 0; i < Words; i++)
    Dst[i] = Src[i];

  uint8       *DstBytes = (uint8 *)(Dst + Words);
  const uint8 *SrcBytes = (const uint8 *)(Src + Words);

  for (uint32 i = 0; i < count % 4; i++)
    DstBytes[i] = SrcBytes[i];

  return dest;
}

void *memmove(void *dest, const void *src, uint32 count)
{
  uint8       *Dst = (uint8 *)dest;
  const uint8 *Src = (const uint8 *)src;

  if (Dst == Src || count == 0)
    return dest;

  if (Dst < Src)
  {
    for (uint32 i = 0; i < count; i++)
      Dst[i] = Src[i];
  }
  else
  {
    for (uint32 i = count; i > 0; i--)
      Dst[i - 1] = Src[i - 1];
  }

  return dest;
}

int memcmp(const void *a, const void *b, uint32 count)
{
  const uint8 *aa = (const uint8 *)a;
  const uint8 *bb = (const uint8 *)b;

  for (uint32 i = 0; i < count; i++)
  {
    if (aa[i] != bb[i])
      return (int)aa[i] - (int)bb[i];
  }

  return 0;
}

char *strchr(const char *str, int character)
{
  while (*str)
  {
    if (*str == (char)character)
      return (char *)str;

    str++;
  }

  if ((char)character == '\0')
    return (char *)str;

  return 0;
}

int strlen(const char *str)
{
  int n = 0;

  while (*str != '\0')
  {
    n++;
    str++;
  }

  return n;
}

char *strcpy(char *dest, const char *src)
{
  char *result = dest;

  while (*src != '\0')
  {
    *dest++ = *src++;
  }

  *dest = '\0';

  return result;
}

int strcmp(const char *String, const char *Other)
{
  int Length1 = strlen(String);
  int Length2 = strlen(Other);

  if (Length1 != Length2)
    return 0;

  int i = 0;
  while (String[i] != '\0')
  {
    if (String[i] != Other[i])
      return 0;

    i++;
  }

  return 1;
}

char *strrchr(const char *str, int character)
{
  const char *Last = 0;

  while (*str)
  {
    if (*str == (char)character)
      Last = str;

    str++;
  }

  if ((char)character == '\0')
    return (char *)str;

  return (char *)Last;
}

static void snprintf_putc(char *buffer, uint32 size, uint32 *position, char c)
{
  if (*position + 1 < size)
    buffer[*position] = c;

  (*position)++;
}

static void snprintf_puts(char *buffer, uint32 size, uint32 *position,
                          const char *str)
{
  while (*str)
  {
    snprintf_putc(buffer, size, position, *str);
    str++;
  }
}

/* Writes `prefix` (e.g. "", "-", "0x") followed by `digits` (digits_len
 * bytes), padded out to `width` total characters. When zero_pad is set the
 * padding is '0' and is inserted *between* the prefix and the digits (so
 * "-7" width 5 zero-padded becomes "-0007", not "000-7"). When left_align
 * is set, padding (always spaces in that case) goes after the content
 * instead of before it. zero_pad is ignored when left_align is set, per
 * normal printf rules. */
static void snprintf_pad_write(char *buffer, uint32 size, uint32 *position,
                               const char *prefix, const char *digits,
                               uint32 digits_len, int width, int zero_pad,
                               int left_align)
{
  uint32 prefix_len = 0;
  while (prefix && prefix[prefix_len])
    prefix_len++;

  int content_len = (int)(digits_len + prefix_len);
  int pad         = width - content_len;

  if (pad < 0)
    pad = 0;

  if (!left_align && !zero_pad)
  {
    for (int i = 0; i < pad; i++)
      snprintf_putc(buffer, size, position, ' ');
  }

  for (uint32 i = 0; i < prefix_len; i++)
    snprintf_putc(buffer, size, position, prefix[i]);

  if (!left_align && zero_pad)
  {
    for (int i = 0; i < pad; i++)
      snprintf_putc(buffer, size, position, '0');
  }

  for (uint32 i = 0; i < digits_len; i++)
    snprintf_putc(buffer, size, position, digits[i]);

  if (left_align)
  {
    for (int i = 0; i < pad; i++)
      snprintf_putc(buffer, size, position, ' ');
  }
}

/* Converts value to text in `out` (no sign, no prefix). Returns the number
 * of digits written. Always writes at least one digit ("0" for value 0). */
static uint32 snprintf_u32_digits(char *out, uint32 value, uint32 base,
                                  int uppercase)
{
  static const char Lower[] = "0123456789abcdef";
  static const char Upper[] = "0123456789ABCDEF";
  const char       *digits  = uppercase ? Upper : Lower;

  if (value == 0)
  {
    out[0] = '0';
    return 1;
  }

  char   temp[32];
  uint32 n = 0;

  while (value != 0)
  {
    temp[n++] = digits[value % base];
    value /= base;
  }

  uint32 i = 0;
  while (n > 0)
    out[i++] = temp[--n];

  return i;
}

static uint32 snprintf_u64_digits(char *out, uint64 value, uint32 base,
                                  int uppercase)
{
  static const char Lower[] = "0123456789abcdef";
  static const char Upper[] = "0123456789ABCDEF";
  const char       *digits  = uppercase ? Upper : Lower;

  if (value == 0)
  {
    out[0] = '0';
    return 1;
  }

  char   temp[32];
  uint32 n = 0;

  while (value != 0)
  {
    temp[n++] = digits[value % base];
    value /= base;
  }

  uint32 i = 0;
  while (n > 0)
    out[i++] = temp[--n];

  return i;
}

int snprintf(char *buffer, uint32 size, const char *format, ...)
{
  uint32 position = 0;

  va_list args;
  va_start(args, format);

  while (*format)
  {
    if (*format != '%')
    {
      snprintf_putc(buffer, size, &position, *format++);
      continue;
    }

    format++;

    if (*format == '\0')
      break;

    /* Flags: '0' zero-pads, '-' left-aligns. Either order, either/both. */
    int ZeroPad   = 0;
    int LeftAlign = 0;

    while (*format == '0' || *format == '-')
    {
      if (*format == '0')
        ZeroPad = 1;
      else
        LeftAlign = 1;

      format++;
    }

    /* Width: a run of decimal digits. */
    int Width = 0;

    while (*format >= '0' && *format <= '9')
    {
      Width = Width * 10 + (*format - '0');
      format++;
    }

    if (*format == '\0')
      break;

    int LongLong = 0;

    if (*format == 'l')
    {
      format++;

      if (*format == 'l')
      {
        LongLong = 1;
        format++;
      }
    }

    if (*format == '\0')
      break;

    switch (*format)
    {
      case '%':
        snprintf_putc(buffer, size, &position, '%');
        break;

      case 'c':
      {
        int  value = va_arg(args, int);
        char c     = (char)value;
        snprintf_pad_write(buffer, size, &position, "", &c, 1, Width, 0,
                           LeftAlign);
        break;
      }

      case 's':
      {
        const char *value = va_arg(args, const char *);

        if (value == 0)
          value = "(null)";

        uint32 len = 0;
        while (value[len] != '\0')
          len++;

        snprintf_pad_write(buffer, size, &position, "", value, len, Width, 0,
                           LeftAlign);
        break;
      }

      case 'd':
      case 'i':
      {
        int         value = va_arg(args, int);
        char        digits[32];
        uint32      len;
        const char *prefix;

        if (value < 0)
        {
          uint32 magnitude = (uint32)(-(value + 1)) + 1;
          len              = snprintf_u32_digits(digits, magnitude, 10, 0);
          prefix           = "-";
        }
        else
        {
          len    = snprintf_u32_digits(digits, (uint32)value, 10, 0);
          prefix = "";
        }

        snprintf_pad_write(buffer, size, &position, prefix, digits, len, Width,
                           ZeroPad, LeftAlign);
        break;
      }

      case 'u':
      {
        char   digits[32];
        uint32 len;

        if (LongLong)
        {
          uint64 value = va_arg(args, uint64);
          len          = snprintf_u64_digits(digits, value, 10, 0);
        }
        else
        {
          uint32 value = va_arg(args, uint32);
          len          = snprintf_u32_digits(digits, value, 10, 0);
        }

        snprintf_pad_write(buffer, size, &position, "", digits, len, Width,
                           ZeroPad, LeftAlign);
        break;
      }

      case 'x':
      case 'X':
      {
        int    uppercase = (*format == 'X');
        char   digits[32];
        uint32 len;

        if (LongLong)
        {
          uint64 value = va_arg(args, uint64);
          len          = snprintf_u64_digits(digits, value, 16, uppercase);
        }
        else
        {
          uint32 value = va_arg(args, uint32);
          len          = snprintf_u32_digits(digits, value, 16, uppercase);
        }

        snprintf_pad_write(buffer, size, &position, "", digits, len, Width,
                           ZeroPad, LeftAlign);
        break;
      }

      case 'p':
      {
        uint32 value = (uint32)(uintptr_t)va_arg(args, void *);
        char   digits[32];
        uint32 len = snprintf_u32_digits(digits, value, 16, 0);

        snprintf_pad_write(buffer, size, &position, "0x", digits, len, Width,
                           ZeroPad, LeftAlign);
        break;
      }

      default:
        snprintf_putc(buffer, size, &position, '%');

        if (LongLong)
        {
          snprintf_putc(buffer, size, &position, 'l');
          snprintf_putc(buffer, size, &position, 'l');
        }

        snprintf_putc(buffer, size, &position, *format);
        break;
    }

    format++;
  }

  va_end(args);

  if (size > 0)
  {
    if (position < size)
      buffer[position] = '\0';
    else
      buffer[size - 1] = '\0';
  }

  return (int)position;
}

char parse_escape(char Character, char Next)
{
  if (Character == '\\')
  {
    switch (Next)
    {
      case '0':
        return '\0';
      case 'a':
        return '\a';
      case 'b':
        return '\b';
      case 'f':
        return '\f';
      case 'n':
        return '\n';
      case 'r':
        return '\r';
      case 't':
        return '\t';
      case 'v':
        return '\v';
      case '\\':
        return '\\';
      case '\'':
        return '\'';
      case '"':
        return '"';
      case '?':
        return '?';
    }
  }

  return Character;
}
