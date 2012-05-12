/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/ide/idelow.c,v 1.13 2000/01/22 10:59:31 naniwa Exp $ */
static char rcsid[] =
    "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/ide/idelow.c,v 1.13 2000/01/22 10:59:31 naniwa Exp $";


#include "ide.h"
#include "../../../include/arch/io.h"


int ide_wait_while_busy()
{
    int stat;
    while ((stat = inb(IDE_STAT_REG)) & IDE_BSY);
    return(stat);
}

int ide_wait_device_ready()
{
    while (!(inb(IDE_STAT_REG) & IDE_DRDY));
}

static int ide_check_ready()
{
    return (inb(IDE_STAT_REG) & IDE_DRDY);
}

static int ide_chk_drq()
{
    return (inb(IDE_STAT_REG) & IDE_DRQ);
}

static int ide_enable_int()
{
    outb(IDE_CONTROL_REG, 0x00);
}

static int ide_disable_int()
{
    outb(IDE_CONTROL_REG, IDE_nIEN);
}

void ide_send_command(int cmd)
{
#ifdef notdef
    ide_check_ready();
    outb(IDE_CONTROL_REG, 0x00);
    ide_wait_while_busy();
#endif
    outb(IDE_COMMAND_REG, cmd);
#ifdef notdef
    /* コマンドの終了はそれぞれのルーチンで待つ */
    ide_wait_while_busy();
    return (inb(IDE_ERROR_REG));
#endif
}



/* read_stat - 
 *
 */
ER read_stat(W drive, struct ide_id * stat)
{
    UH *p;
    W i, j;
    W N, status;

#ifdef notdef
    dbg_printf("IDE:read_stat start\n");
#endif
    for (j = 0; j < IDE_RETRY; ++j) {
	ide_wait_while_busy();
	outb(IDE_DRIVE_REG, (drive << DRIVE_SHIFT) | 0);
	ide_wait_while_busy();
	ide_wait_device_ready();
	ide_send_command(IDE_CMD_READ_INFO);
	ide_wait_while_busy();
	ide_wait_device_ready();
	while (!ide_chk_drq());
	p = (unsigned short *) stat;
	for (i = 0; i < sizeof(struct ide_id) / sizeof(short int); i++) {
	    *p = inw(IDE_DATA_REG);
	    p++;
	}
	status = ide_wait_while_busy();

	if (!(status & IDE_ERR))
	    break;
    }
    if (j == IDE_RETRY) {
	dbg_printf("[IDE] can not read device ID\n");
	return (E_SYS);
    }
#ifdef notdef
    printf("IDE ID %d\n", sizeof(struct ide_id));
    printf("[%s]\n", stat->firm_ware);
    printf("  CYL:  %d(%d)\n", stat->n_cylinder, stat->now_cylinder);
    printf("  HEAD: %d(%d)\n", stat->n_head, stat->now_head);
    printf("  SEC:  %d(%d)\n", stat->n_sector, stat->now_sector);
#endif

#ifdef notdef
    N = calc_N(stat->n_cylinder);
    stat->now_cylinder = (stat->n_cylinder) / N;
    stat->now_head = (stat->n_head) * N;
    stat->now_sector_track = (stat->n_sector);
#endif

    return (E_OK);
}

#ifdef notdef
calc_N(int cylinder)
{
    int N, N2;

    if (cylinder > 1024) {
	cylinder = ROUNDUP(cylinder, 1024);
	N = ((cylinder - 1) / 1024) + 1;

	for (N2 = 1; N2 < N; N2 = N2 * 2) {
	    ;
	}
	return (N2);
    }

    return (1);
}
#endif

ER get_data(W drive, UW head, UW cyl, UW sec, B * buf, W length)
{
    W stat;
    W i;
    UH *tmpbuf;

    length *= IDE_BLOCK_SIZE;

    ide_intr_flag = FALSE;
    ide_wait_while_busy();

    dis_int();
    outb(IDE_DRIVE_REG, (drive << DRIVE_SHIFT) | head);
    ide_wait_while_busy();
    ide_wait_device_ready();
    outb(IDE_SCOUNT_REG, 1);
    outb(IDE_CYL_LOW_REG, cyl & 0xff);
    outb(IDE_CYL_HIGH_REG, (cyl >> CYLINDER_SHIFT) & 0xff);
    outb(IDE_SNUMBER_REG, sec & 0xff);

    ide_send_command(IDE_CMD_READ);
    ena_int();

#if 0
    if (!(inb(IDE_STAT_REG) & IDE_DRQ)) {
#ifdef notdef
	wait_int(&ide_intr_flag);
#endif
    }
#endif

    stat = ide_wait_while_busy();

    if (stat & IDE_ERR) {
	dbg_printf("IDE read error. stat = 0x%x\n", inb(IDE_ERROR_REG));
	return (0);
    }

    while(! ide_chk_drq());

    tmpbuf = (unsigned short *) buf;
    for (i = 0; i < length / sizeof(short int); i++) {
	tmpbuf[i] = inw(IDE_DATA_REG);
    }

    /* 読み込み終了チェック */
    stat = ide_wait_while_busy();

    if (stat & IDE_ERR) {
	dbg_printf("[IDE] read error. stat = 0x%x\n", inb(IDE_ERROR_REG));
	return (0);
    }
#ifdef notdef
    ide_enable_int();
#endif
    return (length);
}

ER put_data(W drive, UW head, UW cyl, UW sec, B * buf, W length)
{
    W stat;
    W i;
    UH *tmpbuf;

    length *= IDE_BLOCK_SIZE;

    ide_intr_flag = FALSE;
    ide_wait_while_busy();

    dis_int();
    outb(IDE_DRIVE_REG, (drive << DRIVE_SHIFT) | head);
    ide_wait_while_busy();
    ide_wait_device_ready();
    outb(IDE_SCOUNT_REG, 1);
    outb(IDE_CYL_LOW_REG, cyl & 0xff);
    outb(IDE_CYL_HIGH_REG, (cyl >> CYLINDER_SHIFT) & 0xff);
    outb(IDE_SNUMBER_REG, sec & 0xff);

    ide_send_command(IDE_CMD_WRITE);
    ena_int();

#if 0
    if (!(inb(IDE_STAT_REG) & IDE_DRQ)) {
#ifdef notdef
	wait_int(&ide_intr_flag);
#endif
    }
#endif

    stat = ide_wait_while_busy();

    if (stat & IDE_ERR) {
	dbg_printf("IDE read error. stat = 0x%x\n", stat);
	return (0);
    }

    while(! ide_chk_drq());

    tmpbuf = (unsigned short *) buf;
    for (i = 0; i < length / sizeof(short int); i++) {
	outw(IDE_DATA_REG, tmpbuf[i]);
    }

    /* 書き込み終了チェック */
    stat = ide_wait_while_busy();

    if (stat & IDE_ERR) {
	dbg_printf("[IDE] write error. stat = 0x%x\n", inb(IDE_ERROR_REG));
	return (0);
    }
#ifdef notdef
    ide_enable_int();
#endif
    return (length);
}

void ide_recalibrate(int drive)
{
    outb(IDE_DRIVE_REG, (drive << DRIVE_SHIFT) | 0);
    ide_wait_while_busy();
    ide_wait_device_ready();
    ide_send_command(IDE_CMD_RECALIBRATE);
    ide_wait_while_busy();
}

/**************************************************************************
 *
 */
int wait_int(int *flag)
{
    ena_int();
    while (*flag == FALSE);
    dis_int();
    *flag = FALSE;
    ena_int();
    return (*flag);
}
