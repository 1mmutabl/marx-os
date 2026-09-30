#include "diskio.h"

#include "drivers/ata/ata.h"
#include "ff.h"

#define FAT32_START_SECTOR 2048

DSTATUS disk_initialize(BYTE pdrv)
{
  if (pdrv != 0)
    return STA_NOINIT;

  ata_init();

  return 0;
}

DSTATUS disk_status(BYTE pdrv)
{
  if (pdrv != 0)
    return STA_NOINIT;

  return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
  uint32 lba;
  uint8  sectors;

  if (pdrv != 0)
    return RES_PARERR;

  if (count == 0)
    return RES_PARERR;

  lba = FAT32_START_SECTOR + (uint32)sector;

  while (count > 0)
  {
    sectors = count > 255 ? 255 : (uint8)count;

    if (ata_read_sectors(lba, sectors, buff) != 0)
      return RES_ERROR;

    lba += sectors;
    buff += (uint32)sectors * 512;
    count -= sectors;
  }

  return RES_OK;
}

#if FF_FS_READONLY == 0

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
  uint32 lba;
  uint8  sectors;

  if (pdrv != 0)
    return RES_PARERR;

  if (count == 0)
    return RES_PARERR;

  lba = FAT32_START_SECTOR + (uint32)sector;

  while (count > 0)
  {
    sectors = count > 255 ? 255 : (uint8)count;

    if (ata_write_sectors(lba, sectors, buff) != 0)
      return RES_ERROR;

    lba += sectors;
    buff += (uint32)sectors * 512;
    count -= sectors;
  }

  return RES_OK;
}

#endif

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
  if (pdrv != 0)
    return RES_PARERR;

  switch (cmd)
  {
    case CTRL_SYNC:
      return RES_OK;

    case GET_SECTOR_SIZE:
      *(WORD *)buff = 512;
      return RES_OK;

    case GET_BLOCK_SIZE:
      *(DWORD *)buff = 1;
      return RES_OK;

    case GET_SECTOR_COUNT:
      *(DWORD *)buff = 131072 - FAT32_START_SECTOR;
      return RES_OK;

    default:
      return RES_PARERR;
  }
}
