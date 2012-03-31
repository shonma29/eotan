#include "config.h"
#include "boot.h"
#include "macros.h"
#include "error.h"
#include "fd.h"
#include "a.out.h"
#include "file.h"
#include "page.h"
#include "memory.h"

/*************************************************************************
 * boot_func --- とりあえずの boot
 *
 * 引数：	なし
 *
 * 返値：	なし
 *
 * 処理：	FD の最初から kernel を読み込む
 *		正式版では、/system/btron386 から読み込むようになる予定
 *
 */
int boot_func(char *argv[])
{
    char tmp[BLOCK_SIZE];
    extern int read_multi_module();

    boot_printf("booting kernel...\n");
    init_fd();
    fd_reset();
    on_motor(0);
    fd_recalibrate(0);
    boot_printf("exec data read...\n");
    fd_read(0, 0, 0, tmp, 2);
    boot_printf("magic number %d\n", tmp[0]);
    if (tmp[0] == 0x0001) {
	boot_printf("read multiple module.\n");
	read_multi_module();
	boot_printf("boot end\n");
    } else {
	boot_printf("a.out module is not supported.\n");
    }

    stop_motor(0);
    return E_OK;
}

/*
   複数のモジュールを読み込む。

   最初のモジュールのみ ITRON カーネルと仮定している。
   そのため、読み込む位置は、0x00010000 と決めている。
   (ITRON カーネルは、仮想アドレス 0x80010000 に読み込むことにしている。
   0x00010000 というのは、0x80010000 に対応している物理アドレスである)

   2 番目以降のモジュールは、ITRON カーネルにの後に続けて読み込むことになる。
   そのため、ITRON カーネルの大きさによって物理アドレスは変わることになる。
   2 番目以降のモジュールについては、boot はロードするだけで、仮想アドレス
   へのマッピングなどは ITRON カーネルの起動後に行う。

*/
int read_multi_module()
{
    int i;
    int bn;
    struct boot_header *info;
    void (*entry) ();
    extern UWORD ticks, clock;

    boot_printf("Multiple module boot.\n");
    info = (struct boot_header *) MODULE_TABLE;
    fd_read(0, 0, 0, (char *) info, 2);
    info->machine.base_mem = base_mem;
    info->machine.ext_mem = ext_mem;
    info->machine.real_mem = real_mem;
    info->machine.rootfs = 0xffffffff;
    boot_printf("Module %d\n", info->count);
    bn = 1;			/* 最初のモジュールが入っているブロック番号 */
    entry = (void (*)()) (info->modules[0].entry);
    for (i = 0; i < info->count; i++) {
	boot_printf("[%d][%s]\n", i, info->modules[i].name);
	read_single_module(bn, (void *) info->modules[i].paddr,
			   &(info->modules[i]));

#ifdef nodef
	bn += ((info->modules[i].length / BLOCK_SIZE) - 1);
#else
	bn += (info->modules[i].length / BLOCK_SIZE + 1);
#endif
    }
    boot_printf("load done.\n");
    boot_printf("exec_info->a_entry = 0x%x\n", entry);
    stop_motor(0);

    info->machine.clock = clock + ticks/TICKS;
    (*entry) ();

    return E_OK;
}

int
read_single_module(int start_block, void *paddr, struct module_info *info)
{
    char buf[BLOCK_SIZE];
    char tmp[BLOCK_SIZE];
    int i, j;
    int bn;
    struct exec *exec_info;

    bn = start_block;
    fd_read(0, 0, bn*2, tmp, 2);
    exec_info = (struct exec *) tmp;
    if (N_BADMAG(*exec_info)) {
	boot_printf("This object is not exec format (%d).\n", *exec_info);
	boot_printf("block number: %d\n", bn);	/* */
	stop_motor(0);
	for (;;);
	/* STOP HERE */
    }

    if ((N_MAGIC(*exec_info) == 0413) || (N_MAGIC(*exec_info) == NMAGIC)) {
	bn += 1;		/* a.out のヘッダの大きさだけカウントアップする */

	boot_printf("blocks: text: %d, data: %d, paddr: 0x%x\n",
		    (ROUNDUP(exec_info->a_text, PAGE_SIZE) / BLOCK_SIZE),
		    (ROUNDUP(exec_info->a_data, PAGE_SIZE) / BLOCK_SIZE),
		    paddr);

	for (i = 0; i < (ROUNDUP(exec_info->a_text, PAGE_SIZE)
			 / BLOCK_SIZE); i++, bn++) {
	    boot_printf(".");
	    fd_read(0, 0, bn*2, buf, 2);
	    bcopy(buf, (char *) (paddr + i * BLOCK_SIZE), BLOCK_SIZE);
	}
/*      boot_printf ("\nText region is readed.\n"); */
	if (exec_info->a_data > 0) {
	    for (j = 0; j <= (ROUNDUP(exec_info->a_data, PAGE_SIZE)
			      / BLOCK_SIZE); j++, bn++) {
		boot_printf(",");
		fd_read(0, 0, bn*2, buf, 2);
		bcopy(buf,
		      (char *) (paddr
				+ (ROUNDUP(exec_info->a_text, PAGE_SIZE))
				+ j * BLOCK_SIZE), BLOCK_SIZE);
	    }
	}
    } else {
	boot_printf("I don't know how to read a.out image.(0x%x)\n",
		    N_MAGIC(*exec_info));
	stop_motor(0);
	for (;;);
    }
/*  boot_printf ("\nload done.\n"); */
    boot_printf("\n");

    return E_OK;
}
