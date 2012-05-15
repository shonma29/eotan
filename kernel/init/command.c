/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
(C) 2000-2002, Tomohide Naniwa

*/

/* $Id: command.c,v 1.17 2000/01/28 10:28:07 monaka Exp $ */
char rcsid[] = "$Id: command.c,v 1.17 2000/01/28 10:28:07 monaka Exp $";

/*
 * $Log: command.c,v $
 * Revision 1.17  2000/01/28 10:28:07  monaka
 * Some hooks for POSIX manager (especially file functions) was deleted.
 * Now Init is more close to POSIX libs.
 *
 * Revision 1.16  2000/01/23 15:56:08  kishida0
 * 下と同じ
 * グラフィック画面スクロール用コマンドはFIX後消す予定
 *
 * Revision 1.15  1999/07/24 04:36:48  naniwa
 * add pmemstat
 *
 * Revision 1.14  1999/07/09 08:26:51  naniwa
 * add print_task_list
 *
 * Revision 1.13  1999/04/13 04:14:47  monaka
 * MAJOR FIXcvs commit -m 'MAJOR FIX!!! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.'! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.
 *
 * Revision 1.12  1999/03/16 02:41:13  monaka
 * Some error messages changed.
 *
 * Revision 1.11  1999/02/17 09:43:12  monaka
 * Modified for 'device descriptor'.
 *
 * Revision 1.10  1998/12/19 03:38:35  monaka
 * Functions for dosfs has commented out
 *
 * Revision 1.9  1998/12/12 05:30:41  monaka
 * Some error trap and some return value added.
 *
 * Revision 1.8  1998/12/12 02:59:39  monaka
 * Minor fix.
 *
 * Revision 1.7  1998/12/10 15:05:58  night
 * open_fd () と vget_region () の引数定義を変更。
 * commands.inc で定義しているものに合わせた。
 *
 * Revision 1.6  1998/12/10 07:17:51  monaka
 * Separated from command.c to posix-file.c and posix-misc.c.
 *
 * Revision 1.5  1998/12/08 08:11:10  monaka
 * Minor change.
 *
 * Revision 1.4  1998/12/08 05:38:55  monaka
 * I init has been included the Mitten Scripting Language since this release.
 * But, it's just a first preview. So you can't use many convenience
 * features. Mitten works extension scripting environment like Tcl, Guile,
 * Windows Scripting Host, and so on. If you wished, you may connect your
 * function written in C to Mitten operator. If you wished, you may call
 * Mitten from C.
 *
 * Revision 1.3  1998/12/02 01:03:49  monaka
 * Major modified for implementing scripting language.
 * But there are a partly connection to user interface.
 *
 * Revision 1.2  1998/11/26 15:24:15  monaka
 *
 * Attributes for built-in commands added.
 * It indicates function call type, the number of argument, and so on.
 *
 * Revision 1.1  1998/11/26 14:02:29  monaka
 *
 * init/command.c has split and moved to some subdirs.
 * This means it was modified how to add a new built-in command.
 *
 * Revision 1.29  1998/07/01 13:48:26  night
 * posix_exec_child() の追加。
 * POSIX exec テスト処理の変更(fork してから exec)。
 *
 * Revision 1.28  1998/06/10 15:55:59  night
 * パーティション情報を出力する ide_geometric() の処理を変更。
 * 各パーティションの CHS 情報も出力していたが、IDE デバイスドライバの処
 * 理の変更(リニアブロック番号で処理するようにした)に伴なって CHS の情報
 * を出力しないようにした。
 *
 * Revision 1.27  1998/02/25 12:32:28  night
 * lowlib_load() の修正。
 * lowlib のプロセス固有データ領域 lowlib_data の
 * 中身を初期化する処理を追加。
 *
 * Revision 1.26  1998/02/24 14:03:56  night
 * pvmdump コマンド (posix_vm_dump()) の修正。
 * 引数の数をチェックするようにした。
 * 引数で指定されたプロセスの情報(仮想ページと実ページの対)
 * を出力するようにした(これまでは、プロセス 0 の情報しか
 * 出力していなかった)。
 *
 * Revision 1.25  1998/02/23 14:41:22  night
 * pvmdump コマンドの追加。
 * 関数 posix_vm_dump の追加。
 *
 * Revision 1.24  1998/02/16 14:10:35  night
 * posix_fork_test 関数の追加。
 * (pfork コマンドの追加)
 *
 * Revision 1.23  1998/01/06 16:35:12  night
 * pexec コマンドの追加。
 * posix_exec_test() 関数の追加。
 *
 * Revision 1.22  1997/10/28 14:45:24  night
 * "ether" コマンドの追加。
 *
 * Revision 1.21  1997/10/18 12:40:13  night
 * print 文を一部変更。
 *
 * Revision 1.20  1997/09/09 13:49:11  night
 * posix ファイルへの書き込みコマンド (pwrite) を追加。
 *
 * Revision 1.19  1997/09/02 14:38:26  night
 * マクロ DOS を追加。
 * このマクロを設定すると、DOS FS を読むコマンドをサポートする。
 *
 * Revision 1.18  1997/08/31 14:03:12  night
 * POSIX/graphic 関係のコマンドを追加。
 *
 * Revision 1.17  1997/07/07 14:43:52  night
 * POSIX 関連の処理を簡潔にした。
 *
 * Revision 1.16  1997/07/07 13:05:00  night
 * writefd の追加。
 * POSIX 関連のコマンドの整理(プリミティブなものは、posix_if.c へ移動した)。
 *
 * Revision 1.15  1997/07/06 11:49:33  night
 * pcat コマンドの処理で、ファイル情報の出力を変更した。
 *
 * Revision 1.14  1997/07/04 15:05:20  night
 * pcat コマンドの追加。
 *
 * Revision 1.13  1997/07/03 14:23:15  night
 * popen システムコールの実行後の結果を表示する内容を変更。
 *
 * Revision 1.12  1997/07/02 13:10:12  night
 * pstatfs コマンドの追加。
 *
 * Revision 1.11  1997/05/14 14:07:28  night
 * popen コマンドの追加
 *
 * Revision 1.10  1997/05/12 14:34:08  night
 * vget_reg システムコールのテスト処理を追加。
 * (コマンド名は vget)
 *
 * Revision 1.9  1997/05/08 15:08:48  night
 * pinit コマンドの追加。
 * このコマンドは POSIX 環境の初期化を行うための処理を入れる予定。
 *
 * Revision 1.8  1997/05/06 12:56:30  night
 * IDE 関係のコマンドを追加。
 *
 * Revision 1.7  1997/04/24 15:34:48  night
 * posix/pmountroot コマンドの追加。
 *
 * Revision 1.6  1996/11/10  11:53:37  night
 * デバッグ文の追加。
 *
 * Revision 1.5  1996/11/07  16:02:32  night
 * pmount コマンドの追加。
 *
 * Revision 1.4  1996/11/07  12:39:29  night
 * reset および falldown コマンドの追加。
 *
 * Revision 1.3  1996/11/06  12:36:45  night
 * カーネルからのコンソールへの出力を直接出力からコンソールデバイスドライ
 * バを介して行うように変更した。
 * そのためのテストコマンド "test" を追加した。
 *
 * Revision 1.2  1996/07/28  19:55:35  night
 * openfd と readfd のコマンド追加。
 *
 * Revision 1.1  1996/07/25  16:01:59  night
 * IBM PC 版用への最初の登録
 *
 * Revision 1.3  1996/01/06 16:00:42  night
 * ヘルプ内容の一部変更。
 *
 * Revision 1.2  1995/12/05 14:32:19  night
 * help(), dos_attach() の関数を追加。
 * dos_ls(), dos_cat() も追加したが、中身はない。
 *
 * Revision 1.1  1995/10/10  16:21:47  night
 * 最初の登録.
 * システム起動後、プロンプトを出力してコマンド入力 - 実行ができるところ
 * まで出来た。ただし、コマンドは echo だけ。
 *
 *
 */
#include "init.h"
#include "lowlib.h"
#include "lib/init-stdlib.h"
#include "../lib/libkernel/libkernel.h"



ER reset(void)
{
    vsys_msc(2, NULL);
    return E_OK;
}

ER falldown(void)
{
    vsys_msc(1, NULL);
    return E_OK;
}

ER lowlib_load(B * name)
{
    ID my;
    ER errno;
    struct lowlib_data lowlib_data;

    printf("loading lowlib: %s\n", name);

    if (get_tid(&my)) {
	printf("lowlib_load: cannot get my taskid.\n");
	return E_SYS;
    }

    errno = lod_low(my, name);
    if (errno) {
	printf("lowlib_load: cannot load lowlib. errno = %d\n", errno);
	return E_SYS;
    }

    /* lowlib が使用するプロセス毎に情報を記憶する領域に情報を設定
     */
    errno =
	vget_reg(my, LOWLIB_DATA, sizeof(struct lowlib_data),
		 &lowlib_data);
    if (errno) {
	printf("lowlib_load: cannot read lowlib data.\n");
	return E_SYS;
    }

    lowlib_data.main_task = my;
    lowlib_data.signal_task = my;
    lowlib_data.my_pid = 0;

    errno =
	vput_reg(my, LOWLIB_DATA, sizeof(struct lowlib_data),
		 &lowlib_data);
    if (errno) {
	printf("lowlib_load: cannot write lowlib data.\n");
	return E_SYS;
    }

    return E_OK;
}

ER print_task_list(void)
{
    vsys_msc(6, NULL);
    return E_OK;
}

ER pmemstat(void)
{
    vsys_msc(7, NULL);
    return E_OK;
}

ER k106jp(void)
{
    DDEV_REQ req;		/* 要求パケット */
    DDEV_RES res;		/* 返答パケット */
    ER error;
    W rsize;
    ID device;
    ID rport;

    error = find_port("driver.keyboard", &device);
    if (error) {
	printf("cannot find port\n");
	return (error);
    }

    rport = get_port(sizeof(res), sizeof(res));
    if (dev_recv == 0) {
	printf("cannot get port\n");
	return (E_NOMEM);
    }

    req.header.mbfid = rport;
    req.header.msgtyp = DEV_CTL;
    req.body.ctl_req.cmd = KEYBOARD_CHANGE_106JP;
    error = snd_mbf(device, sizeof(req), &req);
    if (error != E_OK) {
	printf("cannot send packet. %d\n", error);	/* */
	return (error);
    }
    printf("kernel: snd_mbf\n");	/* */
    rsize = sizeof(res);
    rcv_mbf((UB *) & res, (INT *) & rsize, rport);
    del_mbf(rport);

    return E_OK;
}

ER k101us(void)
{
    DDEV_REQ req;		/* 要求パケット */
    DDEV_RES res;		/* 返答パケット */
    ER error;
    W rsize;
    ID device;
    ID rport;

    error = find_port("driver.keyboard", &device);
    if (error) {
	printf("cannot find port\n");
	return (error);
    }

    rport = get_port(sizeof(res), sizeof(res));
    if (dev_recv == 0) {
	printf("cannot get port\n");
	return (E_NOMEM);
    }

    req.header.mbfid = rport;
    req.header.msgtyp = DEV_CTL;
    req.body.ctl_req.cmd = KEYBOARD_CHANGE_101US;
    error = snd_mbf(device, sizeof(req), &req);
    if (error != E_OK) {
	printf("cannot send packet. %d\n", error);	/* */
	return (error);
    }
    printf("kernel: snd_mbf\n");	/* */
    rsize = sizeof(res);
    rcv_mbf((UB *) & res, (INT *) & rsize, rport);
    del_mbf(rport);

    return E_OK;
}
