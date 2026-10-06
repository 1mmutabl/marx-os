#include <drivers/ata/ata.h>
#include <drivers/keyboard/keyboard.h>
#include <drivers/mouse/mouse.h>
#include <fs/fatfs/ff.h>
#include <fs/fs.h>
#include <lib/heap.h>
#include <lib/string.h>
#include <types.h>

FATFS FileSystem;

FRESULT fs_init(void)
{
  ata_init();
  FRESULT Result = f_mount(&FileSystem, "", 1);

  file_create("/system/input/keyboard.sys");
  file_create("/system/input/mouse.sys");

  return Result;
}

FRESULT file_create(const char *Path)
{
  char Buffer[256];
  int  Length = strlen(Path);

  if (Length >= sizeof(Buffer))
    return FR_INVALID_NAME;

  strcpy(Buffer, Path);

  char *LastSlash = 0;

  for (int i = 0; i < Length; i++)
  {
    if (Buffer[i] == '/')
      LastSlash = &Buffer[i];
  }

  if (LastSlash)
  {
    *LastSlash = '\0';

    if (Buffer[0] != '\0')
    {
      FRESULT Result = dir_create(Buffer);

      if (Result != FR_OK)
        return Result;
    }
  }

  FIL File;

  FRESULT Result = f_open(&File, Path, FA_CREATE_ALWAYS);

  if (Result != FR_OK)
    return Result;

  f_close(&File);
  return FR_OK;
}

uint8 file_exists(const char *Path)
{
  FILINFO Info;
  FRESULT Result = f_stat(Path, &Info);

  return Result == FR_OK;
}

FRESULT file_write(const char *Path, const void *Write, int Size)
{
  if (strcmp(Path, "/system/input/keyboard.sys"))
    return FR_INVALID_PARAMETER;
  else if (strcmp(Path, "/system/input/mouse.sys"))
    return FR_INVALID_PARAMETER;

  FIL File;

  FRESULT Result = f_open(&File, Path, FA_CREATE_ALWAYS | FA_WRITE);

  if (Result == FR_NO_PATH)
  {
    file_create(Path);
    Result = f_open(&File, Path, FA_CREATE_ALWAYS | FA_WRITE);
  }

  if (Result != FR_OK)
    return Result;

  UINT Written;
  Result = f_write(&File, Write, Size, &Written);

  f_close(&File);

  return Result;
}

FRESULT file_read(const char *Path, void *Read, int Size)
{
  if (!Path || !Read || Size < 0)
    return FR_INVALID_PARAMETER;

  if (strcmp(Path, "/system/input/keyboard.sys"))
  {
    if (Size != sizeof(kb_event))
      return FR_INVALID_PARAMETER;

    keyboard_get((kb_event *)Read);
    return FR_OK;
  }
  else if (strcmp(Path, "/system/input/mouse.sys"))
  {
    if (Size != sizeof(ms_event))
      return FR_INVALID_PARAMETER;

    mouse_get((ms_event *)Read);
    return FR_OK;
  }

  FIL File;

  FRESULT Result = f_open(&File, Path, FA_READ);
  if (Result != FR_OK)
    return Result;

  UINT ReadSize = 0;

  Result = f_read(&File, Read, (UINT)Size, &ReadSize);

  FRESULT CloseResult = f_close(&File);

  if (Result != FR_OK)
    return Result;

  if (CloseResult != FR_OK)
    return CloseResult;

  if (ReadSize != (UINT)Size)
    return FR_INT_ERR;

  return FR_OK;
}

int file_size(const char *Path)
{
  FILINFO Info;

  if (f_stat(Path, &Info) == FR_OK)
    return Info.fsize;

  return 0;
}

FRESULT file_append(const char *Path, const void *Write, int Size)
{
  FIL File;

  FRESULT Result = f_open(&File, Path, FA_OPEN_ALWAYS | FA_WRITE);

  if (Result == FR_NO_PATH)
  {
    file_create(Path);
    Result = f_open(&File, Path, FA_OPEN_ALWAYS | FA_WRITE);
  }

  if (Result != FR_OK)
    return Result;

  Result = f_lseek(&File, f_size(&File));

  if (Result != FR_OK)
  {
    f_close(&File);
    return Result;
  }

  UINT Written;
  Result = f_write(&File, Write, Size, &Written);

  f_close(&File);
  return Result;
}

FRESULT file_read_at(const char *Path, int Offset, void *Read, int Size)
{
  FIL     File;
  FRESULT Result = f_open(&File, Path, FA_READ);

  if (Result != FR_OK)
    return Result;

  Result = f_lseek(&File, Offset);

  if (Result != FR_OK)
  {
    f_close(&File);
    return Result;
  }

  UINT ReadSize;
  Result = f_read(&File, Read, Size, &ReadSize);

  f_close(&File);
  return Result;
}

FRESULT file_truncate(const char *Path, int NewSize)
{
  FIL     File;
  FRESULT Result = f_open(&File, Path, FA_OPEN_EXISTING | FA_WRITE);

  if (Result != FR_OK)
    return Result;

  Result = f_lseek(&File, NewSize);

  if (Result != FR_OK)
  {
    f_close(&File);
    return Result;
  }

  Result = f_truncate(&File);

  f_close(&File);
  return Result;
}

FRESULT file_write_at(const char *Path, int Offset, const void *Write, int Size)
{
  if (strcmp(Path, "/system/input/keyboard.sys"))
    return FR_INVALID_PARAMETER;

  FIL     File;
  FRESULT Result;

  if (Offset < 0 || Size < 0)
    return FR_INVALID_PARAMETER;

  Result = f_open(&File, Path, FA_OPEN_ALWAYS | FA_WRITE);

  if (Result == FR_NO_PATH)
  {
    Result = file_create(Path);

    if (Result != FR_OK)
      return Result;

    Result = f_open(&File, Path, FA_OPEN_ALWAYS | FA_WRITE);
  }

  if (Result != FR_OK)
    return Result;

  Result = f_lseek(&File, Offset);

  if (Result != FR_OK)
  {
    f_close(&File);
    return Result;
  }

  UINT Written;

  Result = f_write(&File, Write, Size, &Written);

  if (Result == FR_OK && Written != (UINT)Size)
    Result = FR_DISK_ERR;

  f_close(&File);

  return Result;
}

uint8 dir_exists(const char *Path)
{
  FILINFO Info;

  if (f_stat(Path, &Info) == FR_OK)
  {
    if (Info.fattrib & AM_DIR)
      return 1;
  }

  return 0;
}

FRESULT dir_create(const char *Path)
{
  char Buffer[256];
  int  Length = strlen(Path);

  if (Length >= sizeof(Buffer))
    return FR_INVALID_NAME;

  strcpy(Buffer, Path);

  for (int i = 0; i < Length; i++)
  {
    if (Buffer[i] == '/')
    {
      if (i == 0)
        continue;

      Buffer[i] = '\0';

      if (!dir_exists(Buffer))
      {
        FRESULT Result = f_mkdir(Buffer);

        if (Result != FR_OK && Result != FR_EXIST)
          return Result;
      }

      Buffer[i] = '/';
    }
  }

  if (!dir_exists(Buffer))
  {
    FRESULT Result = f_mkdir(Buffer);

    if (Result != FR_OK && Result != FR_EXIST)
      return Result;
  }

  return FR_OK;
}

unsigned int dir_size(const char *Path)
{
  DIR          Dir;
  FILINFO      Info;
  unsigned int Count = 0;

  if (f_opendir(&Dir, Path) != FR_OK)
    return 0;

  while (f_readdir(&Dir, &Info) == FR_OK && Info.fname[0] != '\0')
    Count++;

  f_closedir(&Dir);
  return Count;
}

FRESULT dir_read(const char *Path, unsigned int DirSize, char *Content[DirSize])
{
  DIR     Dir;
  FILINFO Info;

  FRESULT Result = f_opendir(&Dir, Path);
  if (Result != FR_OK)
    return Result;

  for (unsigned int i = 0; i < DirSize; i++)
    Content[i] = NULL;

  for (unsigned int i = 0; i < DirSize; i++)
  {
    Result = f_readdir(&Dir, &Info);

    if (Result != FR_OK)
      break;

    if (Info.fname[0] == '\0')
      break;

    size_t PathLength = strlen(Path);
    size_t NameLength = strlen(Info.fname);

    size_t Length;

    if (strcmp(Path, "/") == 0)
      Length = PathLength + NameLength + 1;
    else
      Length = PathLength + 1 + NameLength + 1;

    Content[i] = kmalloc(Length);

    if (Content[i] == NULL)
    {
      Result = FR_NOT_ENOUGH_CORE;
      break;
    }

    if (Info.fattrib & AM_DIR)
      snprintf(Content[i], Length, "%s/", Info.fname);
    else
      snprintf(Content[i], Length, "%s", Info.fname);
  }

  f_closedir(&Dir);
  return Result;
}
