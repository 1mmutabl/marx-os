#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>

#define stdin "stdin"
#define stdout "stdout"
#define stderr "stderr"

// Temporary
int f_exists(const char *Path)
{
  return 1;
}

// Temporary
void f_write(const char *Path, const void *Write, unsigned long long Size)
{
}

int atoi(const char *Buffer)
{
  const char *Temp   = Buffer;
  int         Result = 0;
  int         Sign   = 1;

  while (*Temp == ' ' || (*Temp >= '\t' && *Temp <= '\r'))
    Temp++;

  if (*Temp == '-')
  {
    Sign = -1;
    Temp++;
  }
  else if (*Temp == '+')
    Temp++;

  while (*Temp != '\0' && *Temp <= '9' && *Temp >= '0')
  {
    Result = (Result * 10) + (*Temp - '0');
    Temp++;
  }

  return Result * Sign;
}

int snprintf(char *restrict Buffer, unsigned long long MaxLength,
             const char *restrict Format, ...)
{
  va_list            Args;
  unsigned long long Written = 0;

  va_start(Args, Format);

  while (*Format != '\0')
  {
    if (*Format == '%')
    {
      Format++;

      if (*Format == '\0')
        break;

      if (*Format == 'c')
      {
        char Ch = (char)va_arg(Args, int);
        if (Written + 1 < MaxLength)
          Buffer[Written] = Ch;

        Written++;
      }
      else if (*Format == 's')
      {
        const char *Str = va_arg(Args, const char *);
        if (Str == 0)
          Str = "(null)";

        while (*Str != '\0')
        {
          if (Written + 1 < MaxLength)
            Buffer[Written] = *Str;

          Written++;
          Str++;
        }
      }
      else if (*Format == 'd' || *Format == 'i')
      {
        int  Num = va_arg(Args, int);
        char NumBuf[32];
        int  Idx = 0;

        if (Num == 0)
          NumBuf[Idx++] = '0';
        else
        {
          unsigned int UnsignedNum = Num;

          if (Num < 0)
          {
            if (Written + 1 < MaxLength)
              Buffer[Written] = '-';

            Written++;
            UnsignedNum = (unsigned int)(-Num);
          }

          while (UnsignedNum > 0)
          {
            NumBuf[Idx++] = (char)('0' + (UnsignedNum % 10));
            UnsignedNum /= 10;
          }
        }

        for (int i = Idx - 1; i >= 0; i--)
        {
          if (Written + 1 < MaxLength)
            Buffer[Written] = NumBuf[i];

          Written++;
        }
      }
      else if (*Format == '%')
      {
        if (Written + 1 < MaxLength)
          Buffer[Written] = '%';

        Written++;
      }
    }
    else
    {
      if (Written + 1 < MaxLength)
        Buffer[Written] = *Format;

      Written++;
    }

    Format++;
  }

  va_end(Args);

  if (MaxLength > 0)
  {
    unsigned long long NullIdx =
        (Written < MaxLength) ? Written : (MaxLength - 1);
    Buffer[NullIdx] = '\0';
  }

  return (int)Written;
}

int fprintf(const char *Path, const char *restrict Format, ...)
{
  va_list            Args;
  char               LocalBuffer[4096];
  unsigned long long MaxLength = sizeof(LocalBuffer);
  unsigned long long Written   = 0;

  va_start(Args, Format);

  while (*Format != '\0')
  {
    if (*Format == '%')
    {
      Format++;

      if (*Format == '\0')
        break;

      if (*Format == 'c')
      {
        char Ch = (char)va_arg(Args, int);
        if (Written + 1 < MaxLength)
          LocalBuffer[Written] = Ch;

        Written++;
      }
      else if (*Format == 's')
      {
        const char *Str = va_arg(Args, const char *);
        if (Str == 0)
          Str = "(null)";

        while (*Str != '\0')
        {
          if (Written + 1 < MaxLength)
            LocalBuffer[Written] = *Str;

          Written++;
          Str++;
        }
      }
      else if (*Format == 'd' || *Format == 'i')
      {
        int  Num = va_arg(Args, int);
        char NumBuf[32];
        int  Idx = 0;

        if (Num == 0)
          NumBuf[Idx++] = '0';
        else
        {
          unsigned int UnsignedNum = Num;

          if (Num < 0)
          {
            if (Written + 1 < MaxLength)
              LocalBuffer[Written] = '-';

            Written++;
            UnsignedNum = (unsigned int)(-Num);
          }

          while (UnsignedNum > 0)
          {
            NumBuf[Idx++] = (char)('0' + (UnsignedNum % 10));
            UnsignedNum /= 10;
          }
        }

        for (int i = Idx - 1; i >= 0; i--)
        {
          if (Written + 1 < MaxLength)
            LocalBuffer[Written] = NumBuf[i];

          Written++;
        }
      }
      else if (*Format == '%')
      {
        if (Written + 1 < MaxLength)
          LocalBuffer[Written] = '%';

        Written++;
      }
    }
    else
    {
      if (Written + 1 < MaxLength)
        LocalBuffer[Written] = *Format;

      Written++;
    }

    Format++;
  }

  va_end(Args);

  if (MaxLength > 0)
  {
    unsigned long long NullIdx =
        (Written < MaxLength) ? Written : (MaxLength - 1);
    LocalBuffer[NullIdx] = '\0';
  }

  if (Written > 0)
  {
    unsigned long long WriteSize =
        (Written < MaxLength) ? Written : (MaxLength - 1);
    f_write(Path, LocalBuffer, WriteSize);
  }

  return (int)Written;
}

int printf(const char *restrict Format, ...)
{
  va_list            Args;
  char               LocalBuffer[4096];
  unsigned long long MaxLength = sizeof(LocalBuffer);
  unsigned long long Written   = 0;

  va_start(Args, Format);

  while (*Format != '\0')
  {
    if (*Format == '%')
    {
      Format++;

      if (*Format == '\0')
        break;

      if (*Format == 'c')
      {
        char Ch = (char)va_arg(Args, int);
        if (Written + 1 < MaxLength)
          LocalBuffer[Written] = Ch;

        Written++;
      }
      else if (*Format == 's')
      {
        const char *Str = va_arg(Args, const char *);
        if (Str == 0)
          Str = "(null)";

        while (*Str != '\0')
        {
          if (Written + 1 < MaxLength)
            LocalBuffer[Written] = *Str;

          Written++;
          Str++;
        }
      }
      else if (*Format == 'd' || *Format == 'i')
      {
        int  Num = va_arg(Args, int);
        char NumBuf[32];
        int  Idx = 0;

        if (Num == 0)
          NumBuf[Idx++] = '0';
        else
        {
          unsigned int UnsignedNum = Num;

          if (Num < 0)
          {
            if (Written + 1 < MaxLength)
              LocalBuffer[Written] = '-';

            Written++;
            UnsignedNum = (unsigned int)(-Num);
          }

          while (UnsignedNum > 0)
          {
            NumBuf[Idx++] = (char)('0' + (UnsignedNum % 10));
            UnsignedNum /= 10;
          }
        }

        for (int i = Idx - 1; i >= 0; i--)
        {
          if (Written + 1 < MaxLength)
            LocalBuffer[Written] = NumBuf[i];

          Written++;
        }
      }
      else if (*Format == '%')
      {
        if (Written + 1 < MaxLength)
          LocalBuffer[Written] = '%';

        Written++;
      }
    }
    else
    {
      if (Written + 1 < MaxLength)
        LocalBuffer[Written] = *Format;

      Written++;
    }

    Format++;
  }

  va_end(Args);

  if (MaxLength > 0)
  {
    unsigned long long NullIdx =
        (Written < MaxLength) ? Written : (MaxLength - 1);
    LocalBuffer[NullIdx] = '\0';
  }

  if (Written > 0)
  {
    unsigned long long WriteSize =
        (Written < MaxLength) ? Written : (MaxLength - 1);
    f_write(stdout, LocalBuffer, WriteSize);
  }

  return (int)Written;
}

#endif
