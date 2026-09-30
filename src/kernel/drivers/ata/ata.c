#include "ata.h"

#include "drivers/io/io.h"

#define ATA_PRIMARY_IO 0x1F0
#define ATA_PRIMARY_CONTROL 0x3F6

#define ATA_REG_DATA 0x1F0
#define ATA_REG_ERROR 0x1F1
#define ATA_REG_FEATURES 0x1F1
#define ATA_REG_SECCOUNT 0x1F2
#define ATA_REG_LBA_LOW 0x1F3
#define ATA_REG_LBA_MID 0x1F4
#define ATA_REG_LBA_HIGH 0x1F5
#define ATA_REG_DRIVE 0x1F6
#define ATA_REG_STATUS 0x1F7
#define ATA_REG_COMMAND 0x1F7

#define ATA_CMD_READ_SECTORS 0x20
#define ATA_CMD_WRITE_SECTORS 0x30
#define ATA_CMD_IDENTIFY 0xEC

#define ATA_STATUS_ERR 0x01
#define ATA_STATUS_DRQ 0x08
#define ATA_STATUS_SRV 0x10
#define ATA_STATUS_DF 0x20
#define ATA_STATUS_RDY 0x40
#define ATA_STATUS_BSY 0x80

static void ata_delay(void)
{
  port_inb(ATA_PRIMARY_CONTROL);
  port_inb(ATA_PRIMARY_CONTROL);
  port_inb(ATA_PRIMARY_CONTROL);
  port_inb(ATA_PRIMARY_CONTROL);
}

static int ata_wait_bsy(void)
{
  uint8 status;

  do
  {
    status = port_inb(ATA_REG_STATUS);

    if (status & ATA_STATUS_ERR)
      return -1;

    if (status & ATA_STATUS_DF)
      return -1;

  } while (status & ATA_STATUS_BSY);

  return 0;
}

static int ata_wait_drq(void)
{
  uint8 status;

  while (1)
  {
    status = port_inb(ATA_REG_STATUS);

    if (status & ATA_STATUS_ERR)
      return -1;

    if (status & ATA_STATUS_DF)
      return -1;

    if (status & ATA_STATUS_DRQ)
      return 0;

    if (!(status & ATA_STATUS_BSY))
      return -1;
  }
}

void ata_init(void)
{
  port_outb(ATA_REG_DRIVE, 0xA0);
  ata_delay();
}

int ata_read_sectors(uint32 lba, uint8 count, void *buffer)
{
  uint8  status;
  uint8 *ptr = (uint8 *)buffer;

  if (count == 0)
    return -1;

  port_outb(ATA_REG_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));

  port_outb(ATA_REG_SECCOUNT, count);

  port_outb(ATA_REG_LBA_LOW, (uint8)(lba & 0xFF));
  port_outb(ATA_REG_LBA_MID, (uint8)((lba >> 8) & 0xFF));
  port_outb(ATA_REG_LBA_HIGH, (uint8)((lba >> 16) & 0xFF));

  port_outb(ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);

  for (uint16 sector = 0; sector < count; sector++)
  {
    if (ata_wait_bsy() != 0)
      return -1;

    if (ata_wait_drq() != 0)
      return -1;

    for (uint16 i = 0; i < 256; i++)
    {
      uint16 data = port_inw(ATA_REG_DATA);

      ptr[0] = (uint8)(data & 0xFF);
      ptr[1] = (uint8)((data >> 8) & 0xFF);

      ptr += 2;
    }
  }

  status = port_inb(ATA_REG_STATUS);

  if (status & ATA_STATUS_ERR)
    return -1;

  if (status & ATA_STATUS_DF)
    return -1;

  return 0;
}

int ata_write_sectors(uint32 lba, uint8 count, const void *buffer)
{
  const uint8 *ptr = (const uint8 *)buffer;

  if (count == 0)
    return -1;

  port_outb(ATA_REG_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));

  port_outb(ATA_REG_SECCOUNT, count);

  port_outb(ATA_REG_LBA_LOW, (uint8)(lba & 0xFF));
  port_outb(ATA_REG_LBA_MID, (uint8)((lba >> 8) & 0xFF));
  port_outb(ATA_REG_LBA_HIGH, (uint8)((lba >> 16) & 0xFF));

  port_outb(ATA_REG_COMMAND, ATA_CMD_WRITE_SECTORS);

  for (uint16 sector = 0; sector < count; sector++)
  {

    if (ata_wait_bsy() != 0)
      return -1;

    if (ata_wait_drq() != 0)
      return -1;

    for (uint16 i = 0; i < 256; i++)
    {
      uint16 data = (uint16)ptr[0] | ((uint16)ptr[1] << 8);

      port_outw(ATA_REG_DATA, data);

      ptr += 2;
    }

    ata_delay();
  }

  if (ata_wait_bsy() != 0)
    return -1;

  port_outb(ATA_REG_COMMAND, 0xE7);

  if (ata_wait_bsy() != 0)
    return -1;

  return 0;
}
