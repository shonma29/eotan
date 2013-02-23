#include <itron/types.h>
#include "types.h"
#include "lib.h"
#include "interrupt.h"
#include "idt.h"
#include "fd.h"
#include "fdc.h"
#include "dma.h"
#include "errno.h"
#include "asm.h"

#define ON	1
#define OFF	0

#define MAX_RETRY	10


struct spec
{
  UWORD32 srt;
  UWORD32 hut;
  UWORD32 hlt;
  UWORD32 nd;
};

struct status
{
  BYTE status_reg;
  BYTE status_data[8];
  WORD32 motor;
};

struct status		fd_status;
static volatile int		intr_flag = FALSE;

static struct spec	fd_spec[] = 
{
  { 0x0d, 0x0f, 0x01, 0x0 }		/* 1.44M */
};

static int motor_goal = 0;
static int motor_status;
static BOOL recalibrate_flag;

/************************************************************************
* on_motor 
*/
int
on_motor (BYTE drive)
{
  int motor_bit, running;

  motor_bit = 1 << drive;		/* 動かしたい FD drive 番号を
					   ビットマップに変換 */
  running = motor_status & motor_bit;	/* nonzero if this motor is running */
  motor_goal = motor_status | motor_bit;/* want this drive running too */

  outb (FDC_WCNTL, (motor_goal << MOTOR_SHIFT) | 0x0c | drive);
  motor_status = motor_goal;

  /* If the motor was not running yet, we have to wait for it. */
  if (!running)
    {
      busywait (1000);
    }

  return E_OK;
}

/************************************************************************
 * stop_motor
 */
int
stop_motor (BYTE drive)
{
  motor_goal = 0;
  if (motor_goal != motor_status) {
	outb (FDC_WCNTL, (motor_goal << MOTOR_SHIFT) | 0x0c);
	motor_status = motor_goal;
  }

  return E_OK;
}

/************************************************************************
 * intr_fd
 * フロッピィディスクの割り込み処理関数
 */
void
intr_fd (void)
{
  clear_int();
  intr_flag = TRUE;
  set_int();
}

/************************************************************************
 * write_fdc, write_commands 
 * fdcにコマンドの書き込みを行う
 */
void
write_fdc (int value)
{
  int status;

  for(;;) {
    status = inb(FDC_STAT);               
    if((status & (FDC_MASTER | FDC_DIN)) == (FDC_MASTER | 0)) {
      outb(FDC_DATA, value);
      break;
    }
  }
}

BOOL
write_commands(int n, BYTE* buff)
{
  int  count;
  int  status;
  int  i;

  for(count = 0; count < 1000; count ++) {

    status = inb(FDC_STAT);                  /* FDCがnon-busyになるのを待つ */
    if((status & FDC_BUSY) == FDC_BUSY)
      continue;
        
    for(i = 0; i < n; i++) {                 /* FDCにコマンドを入力する */
      write_fdc(buff[i]);
    }
    return TRUE;                             /* 正常終了 */
  }
  return FALSE;                              /* 時間切れ */ 
}

/************************************************************************
 * init_fd
 */
int
init_fd (void)
{
  set_idt (38, 0x08, (int)int38_handler, INTERRUPT_DESC, 0);
  reset_intr_mask (6);

  fd_reset();
  recalibrate_flag = FALSE;	            
	
  return E_OK;
}

/************************************************************************
 *fd_reset
 */
int
fd_reset (void)
{
  fd_specify (fd_spec[HD_TYPE].srt,
	      fd_spec[HD_TYPE].hut,
	      fd_spec[HD_TYPE].hlt,
	      fd_spec[HD_TYPE].nd);

  return TRUE;
}

/************************************************************************
 * fd_specify ---
 */

int
fd_specify (UWORD32 srt, UWORD32 hut, UWORD32 hlt, UWORD32 nd)
{
  BYTE cbuff[3];

  cbuff[0] = FDC_SPECIFY;
  cbuff[1] = ((srt << 4) | (hut & 0x0f)) & 0xff;
  cbuff[2] = ((hlt << 1) | (nd & 0x01)) & 0xff;
  write_commands(3, cbuff);
  
  outb(FDC_DCR, 0x00);
  return TRUE;
}

/************************************************************************
 * fd_recalibrate
 */

int
fd_recalibrate (BYTE drive)
{
  BYTE cbuff[2];
  
  intr_flag = FALSE;	                        /* 割り込み待ち */

  cbuff[0] = FDC_RECALIBRATE;                   /* リキャリブレート */ 
  cbuff[1] = drive;
  write_commands(2, cbuff);

  wait_int (&intr_flag);               
  fdc_isense ();                                /* 実行結果の受取 */

  return (fd_status.status_data[0]);
}

/************************************************************************
 * fd_seek
 */

int
fd_seek (BYTE drive, int head, int cylinder, int motor)
{
  int	result;
  BYTE cbuff[3];

  if(recalibrate_flag == FALSE) {               /* 初回の一度だけリキャリブレートする */
    fd_recalibrate(drive);
    recalibrate_flag = TRUE;
  }

  intr_flag = FALSE;                            /* 割り込み待ち */

  cbuff[0] = FDC_SEEK;                          /* シーク */
  cbuff[1] = (head << 2) | (drive & 0x03);
  cbuff[2] = cylinder;
  write_commands(3, cbuff);
  
  wait_int (&intr_flag);
  fdc_isense ();                                /* 実行結果の受取 */

  result = TRUE;
  if ((fd_status.status_data[0] & 0xF8) != 0x20) 
    result = FALSE;

  if (fd_status.status_data[1] != cylinder) {
    result = FALSE;
  }   
  return (result);
}

/************************************************************************
 * fdc_isense
 * 
 */

int
fdc_isense (void)
{
  int	result_nr = 0;
  int	status;
  BYTE  cbuff[1];

  cbuff[0] = FDC_SENSE;
  write_commands(1, cbuff);

  status = inb (FDC_STAT);
  for (;;)
    {
      if (status == (FDC_MASTER | FDC_DIN | FDC_BUSY)) 
	{
	  if (result_nr >= 8) 
	    break;	/* too many results */
	  fd_status.status_data[result_nr++] = inb (FDC_DATA);
	}
      status = inb (FDC_STAT) & (FDC_MASTER | FDC_DIN | FDC_BUSY);
      if (status == FDC_MASTER) 
	{	/* all read */
	  return (TRUE);
	}
    }
  return (FALSE);
}
/*************************************************************************
 * fdc_sense
 * 
 */

int
fdc_sense (void)
{
  int	result_nr = 0;
  int	status;

  status = inb (FDC_STAT);
  for (;;)
    {
      if (status == (FDC_MASTER | FDC_DIN | FDC_BUSY)) 
	{
	  if (result_nr >= 8) 
	    break;	/* too many results */
	  fd_status.status_data[result_nr++] = inb (FDC_DATA);
	}
      status = inb (FDC_STAT) & (FDC_MASTER | FDC_DIN | FDC_BUSY);
      if (status == FDC_MASTER) 
	{	/* all read */
	  return (TRUE);
	}
    }
  return (FALSE);
}

/************************************************************************
 * fd_read_sector
 * cylinder, head, sector で示される位置から 1 セクター buff に読み込む
 */
int
fd_read_sector(BYTE drive, int cylinder, int head, int sector, BYTE* buff)
{
  int i;
  BYTE cbuff[9];
  int s;

  for(i = 0; i < MAX_RETRY; i++) {
    if(fd_seek(drive, head, cylinder, 0) == FALSE)               /* シーク */
	continue;
    
    setup_dma((void*)FD_DMA_BUFF, DMA_READ, HD_LENGTH, DMA_MASK);      /* DMA設定 */
    
    intr_flag = FALSE;

    cbuff[0] = FDC_READ;                                        /* リード */ 
    cbuff[1] = (head << 2) | drive;
    cbuff[2] = cylinder;
    cbuff[3] = head;
    cbuff[4] = sector;
    cbuff[5] = HD_LENCODE;
    cbuff[6] = HD_SECTOR;
    cbuff[7] = HD_GAP;
    cbuff[8] = HD_DTL;
    write_commands(9, cbuff);
    
    wait_int (&intr_flag);                                      /* 割り込み待ち */

    if(fdc_sense () == FALSE)                                   /* エラーチェック */
      continue;
    if ((fd_status.status_data[0] & 0xF8) != 0x00) 
      continue;
    if ((fd_status.status_data[1] | fd_status.status_data[2]) != 0x00)
      continue;

    s = fd_status.status_data[3] * HD_HEAD * HD_SECTOR                   /* 読み込んだセクター数を計算 */
      + fd_status.status_data[4] * HD_SECTOR + fd_status.status_data[5]; 
    s = s - (cylinder * HD_HEAD * HD_SECTOR + head * HD_SECTOR + sector);     
    if (s  != 1) 
      continue;
    
    bcopy((void*)FD_DMA_BUFF, buff, HD_LENGTH);
    return TRUE;                                                /* 正常終了 */ 
  }

  return FALSE;                                                 /* 異常終了 */
}

/************************************************************************
 * fd_read
 * blockno で示される位置から length ブロック buff に読み込む 
 */
int
fd_read (int drive, int part, int blockno, BYTE *buff, int length)
{
  /* part はダミー */


  int	cylinder;
  int	head;
  int	sector;
  int	i;
  BOOL	ret;
  int	readcount;

  for (readcount = 0; readcount < length; readcount++) {
 
    head = (blockno % (HD_HEAD * HD_SECTOR)) / HD_SECTOR;   
    cylinder = (blockno / (HD_HEAD * HD_SECTOR));           
    sector = (blockno % HD_SECTOR) + 1;

    ret = FALSE;
    for (i = 0; i < MAX_RETRY; i++) {
      if(fd_read_sector(drive, cylinder, head, sector, buff) == FALSE)
	continue;
      else {
	ret = TRUE;
	break;
      }
    }
    if(ret == FALSE) {
      boot_printf ("Read failed.\n");
      return -1;
    }

    blockno = blockno + 1;
    buff = buff + HD_LENGTH;
  }

  return 0;
}




