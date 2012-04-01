#include "types.h"
#include "macros.h"
#include "config.h"
#include "boot.h"
#include "console.h"
#include "memory.h"
#include "interrupt.h"
#include "idt.h"
#include "dma.h"
#include "errno.h"
#include "file.h"
#include "page.h"
#include "a.out.h"
#include "asm.h"
#include "lib.h"
#include "ide.h"
#include "date.h"

int ide_read_low (int drive, int cyl_b, int head_b, int sec_b, char *buf, int length);
int read_partition_table (int drive, struct ide_partition *table);

int			ide_intr_flag;
struct ide_id		ide_parameter;
struct ide_partition	partition_table[IDE_MAX_PARTITION+IDE_MAX_EXT_PARTITION];
int			ext_partition;

void
intr_ide ()
{
  boot_printf ("IDE:interrupt!\n");
  clear_int ();
  ide_intr_flag = TRUE;
  set_int ();
}

static int
ide_check_ready ()
{
  int result;
  result = inb (IDE_STAT_REG) & 0x40; 
  return result;
}

static int
ide_wait_while_busy ()
{
  while (inb (IDE_STAT_REG) & 0x80)
    {
      busywait (5);
    }

  return E_OK;
}

static int
ide_chk_drq ()
{
  return (inb (IDE_STAT_REG) & 0x08);
}

static int
ide_enable_int ()
{
  outb (IDE_CONTROL_REG, 0x00);

  return E_OK;
}

static int
ide_send_command (BYTE cmd)
{
  while (!ide_check_ready ())
    ;
  ide_wait_while_busy ();

  outb (IDE_COMMAND_REG, cmd & 0xff);
  ide_wait_while_busy ();
  return (inb (IDE_ERROR_REG));
}


void
ide_id (void)
{
  int i;

  get_ide_parameter (&ide_parameter);

  for (i = 0; i < 20; i++)
    {
      boot_printf ("%c", ide_parameter.model[i * 2 + 1]);
      boot_printf ("%c", ide_parameter.model[i * 2]);
    }
  boot_printf ("\n");
  boot_printf ("C/H/S: %d/%d/%d\n", 
	  ide_parameter.n_cylinder,
	  ide_parameter.n_head,
	  ide_parameter.n_sector);

  for (i = 0; i < IDE_MAX_PARTITION; i++)
    {
      boot_printf ("partition[%d] type = 0x%x, start = %d, length = %d, bootable = %d\n",
	      i, 
	      partition_table[i].type,
	      (int)partition_table[i].start,
	      (int)partition_table[i].length,
	      (int)partition_table[i].boot_flag);
    }

}

void
ide_init (void)
{
  int	status;

  outb (IDE_DRIVE_REG, 0xA0);			/* ドライブ 0 を指定 */

  outb (IDE_CONTROL_REG, 0x08);			/* ドライブのリセット */
  busywait (1000);

  outb (IDE_CONTROL_REG, 0x0a);
  busywait (1000);

  status = inb (IDE_STAT_REG);
  boot_printf ("IDE init status = 0x%x\n", status);

  get_ide_parameter (&ide_parameter);

  read_partition_table (0, partition_table);
}

/* IDE HD のテスト関数 
 *
 * ディスクパラメータを出力する
 */
int get_ide_parameter (struct ide_id *id)
{
  int	i;
  UWORD16 *p;

  clear_int ();
  outb (IDE_DRIVE_REG, 0xA0);			/* ドライブ 0 を指定 */
  busywait (1000);
  ide_send_command (IDE_CMD_READ_INFO);
  p = (UWORD16 *)id;
  if (ide_chk_drq ())
    {
      for (i = 0; i < sizeof (struct ide_id) / 2; i++)
	{
	  *p = inw (IDE_DATA_REG);
	  p++;
	}
    }
  else
    {
      boot_printf ("drive busy. (%d)\n", __LINE__);
    }

  set_int ();

  boot_printf ("\n");

  return E_OK;
}


int
ide_read_low (int drive, int cyl_b, int head_b, int sec_b, char *buf, int length)
{
  int	stat;
  int	i;
  UWORD16 *tmpbuf;
  int	cyl, head, sec;

  cyl = cyl_b;
  head = head_b;
  sec = sec_b;

  length *= IDE_BLOCK_SIZE;

  ide_intr_flag = FALSE;
  while (inb (IDE_STAT_REG) & 0x80)
    ;

  clear_int ();

  outb (IDE_DRIVE_REG, 0xA0 | ((drive << 4) | head));
  outb (IDE_SCOUNT_REG, 1);
  outb (IDE_CYL_LOW_REG, cyl & 0xff);
  outb (IDE_CYL_HIGH_REG, (cyl >> 8) & 0xff);
  outb (IDE_SNUMBER_REG, sec & 0xff);

  ide_send_command (IDE_CMD_READ);
  set_int ();

  while ((stat = inb (IDE_STAT_REG)) & 0x80)
    ;

  if (stat & 0x01)
    {
      boot_printf ("IDE read error.\n");
      return (0);
    }

  while (!(inb (IDE_STAT_REG) & IDE_DRQ))
    {
      busywait (1);
    }

  tmpbuf = (UWORD16 *)buf;
  for (i = 0; i < length / 2; i++)
    {
      tmpbuf[i] = inw (IDE_DATA_REG);
    }

  ide_enable_int (drive);
  return (length);
}


int
read_partition_table (int drive, struct ide_partition *table)
{
  BYTE buf[IDE_BLOCK_SIZE];
  int	i, have_ext_partition;
  struct ide_partition pt_buf[IDE_MAX_PARTITION], *tp;
  unsigned int ext_offset;

  ide_read_low (0, 0, 0, 1, buf, 1);
  bcopy (&buf[PARTITION_OFFSET], (char *)table, sizeof (struct ide_partition) * 4);

  ext_partition = 0;
  have_ext_partition = -1;
  for (i = 0; i < IDE_MAX_PARTITION; i++)
    {
      boot_printf ("partition[%d] type = 0x%x, start = %d, length = %d, bootable = %d\n",
	      i + 1, 
	      table[i].type,
	      table[i].start,
	      table[i].length,
	      table[i].boot_flag);
#ifdef notdef
      boot_printf ("begin(CHS): %d/%d/%d\n",
	      table[i].begin[2] | ((table[i].begin[1] & 0xc0) << 2),
	      table[i].begin[0],
	      table[i].begin[1] & 0x3f);
      boot_printf ("end(CHS): %d/%d/%d\n",
	      table[i].end[2] | ((table[i].end[1] & 0xc0) << 2),
	      table[i].end[0],
	      table[i].end[1] & 0x3f);
#endif
      switch(table[i].type) {
      case 0x05:
      case 0x0F:
      case 0x85:
	have_ext_partition = i;
	break;
      }
    }

  if (have_ext_partition >= 0) {
    ext_offset = partition_table[have_ext_partition].start;
    table[IDE_MAX_PARTITION].start = ext_offset;
    do {
      ide_read(0, have_ext_partition, 0, buf, 1);
      bcopy (&buf[PARTITION_OFFSET], (char *) &pt_buf,
	     sizeof (struct ide_partition) * 4);
#ifdef notdef
      boot_printf ("partition[%d] type = 0x%x, start = %d, length = %d, bootable = %d\n",
		   IDE_MAX_PARTITION + ext_partition, 
		   pt_buf[0].type,
		   pt_buf[0].start,
		   pt_buf[0].length,
		   pt_buf[0].boot_flag);
#endif
      tp = &table[IDE_MAX_PARTITION + ext_partition];
      tp->type = pt_buf[0].type;
      tp->start += pt_buf[0].start;
      tp->length = pt_buf[0].length;
      tp->boot_flag = pt_buf[0].boot_flag;
      ++ext_partition;
      if (ext_partition < IDE_MAX_EXT_PARTITION) {
	++tp; 
	tp->start = pt_buf[1].start + ext_offset;
      }
      have_ext_partition = IDE_MAX_PARTITION + ext_partition;
    } while (pt_buf[1].type && ext_partition < IDE_MAX_EXT_PARTITION);
  }

  for (i = 0, tp = &table[IDE_MAX_PARTITION]; i < ext_partition; ++i, ++tp) {
    boot_printf ("partition[%d] type = 0x%x, start = %d, length = %d, bootable = %d\n",
		 IDE_MAX_PARTITION + i + 1, 
		 tp->type, tp->start, tp->length, tp->boot_flag);
  }
  return E_OK;
}

int
ide_read_block_1k (int drive, int partition, unsigned int block, BYTE *buf, int length)
{
  int	err;
  err = ide_read (drive, partition, block , buf, length);
  return (err);
}


int
ide_read (int drive, int partition, ULONG block, BYTE *buf, int length)
{
  unsigned int	true_block;
  int		i;
  int		cylinder, sector, head;

  if ((block > partition_table[partition].length) || (block < 0))
    {
      boot_printf ("Block numer is invalid (blockno = %d)\n", (int)block);
      boot_printf ("Total block of partition is %d\n", partition_table[partition].length);
      boot_printf ("Buffer address = 0x%x, length = %d\n", (int)buf, length);
      for(;;);
      return (E_PAR);
    }

  true_block = partition_table[partition].start + block;

  if (true_block < 0)
    {
      int	i;

      boot_printf ("ERROR!! read block number is invalid!.\n");
      boot_printf ("true_block = %d, partition = %d, startblock = %d, block = %d\n",	/*  */
	      true_block, partition, partition_table[partition].start, block);
      boot_printf ("Buffer address = 0x%x, length = %d\n", buf, length);

      for (i = 0; i < IDE_MAX_PARTITION; i++)
	{
	  boot_printf ("partition[%d] type = 0x%x, start = %d, length = %d, bootable = %d\n",
		  i, 
		  partition_table[i].type,
		  partition_table[i].start,
		  partition_table[i].length,
		  partition_table[i].boot_flag);
	  boot_printf ("begin(CHS): %d/%d/%d\n",
		  partition_table[i].begin[2] | ((partition_table[i].begin[1] & 0xc0) << 2),
		  partition_table[i].begin[0],
		  partition_table[i].begin[1] & 0x3f);
	  boot_printf ("end(CHS): %d/%d/%d\n",
		  partition_table[i].end[2] | ((partition_table[i].end[1] & 0xc0) << 2),
		  partition_table[i].end[0],
		  partition_table[i].end[1] & 0x3f);
	}
      for (;;)
	;
      return (E_PAR);
    }

  for (i = 0; i < length; i++)
    {
#ifndef USE_LBA
      cylinder = (true_block + i) / (ide_parameter.n_head * ide_parameter.n_sector);
      head =     ((true_block + i) % (ide_parameter.n_head * ide_parameter.n_sector)) / ide_parameter.n_sector;
      sector =   ((true_block + i) % ide_parameter.n_sector) + 1;
#else
      sector = LBA_8(true_block + i);
      cylinder = LBA_16(true_block + i);
      head = LBA_4(true_block + i);
      drive |= LBA_MODE_BIT;
#endif
      ide_read_low (drive, cylinder, head, sector, (buf + (i  * IDE_BLOCK_SIZE)), 1);
    }

  return (E_OK);
}
