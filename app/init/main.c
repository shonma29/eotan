/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2000-2003, Tomohide Naniwa

*/
/* $Id: main.c,v 1.15 2000/01/15 15:17:02 naniwa Exp $ */

/*
 * $Log: main.c,v $
 * Revision 1.15  2000/01/15 15:17:02  naniwa
 * no more depend on libc.a
 *
 * Revision 1.14  1999/11/10 10:23:57  naniwa
 * to support execve
 *
 * Revision 1.13  1999/05/10 02:56:04  naniwa
 * add read_line_edit()
 *
 * Revision 1.12  1999/03/30 13:15:23  monaka
 * Now we connect to lowlib.posix by default.
 *
 * Revision 1.11  1999/03/15 06:00:45  monaka
 * modified some debug messages.
 *
 * Revision 1.10  1999/02/17 09:43:09  monaka
 * Modified for 'device descriptor'.
 *
 * Revision 1.9  1998/12/08 05:38:35  monaka
 * I init has been included the Mitten Scripting Language since this release.
 * But, it's just a first preview. So you can't use many convenience
 * features. Mitten works extension scripting environment like Tcl, Guile,
 * Windows Scripting Host, and so on. If you wished, you may connect your
 * function written in C to Mitten operator. If you wished, you may call
 * Mitten from C.
 *
 * Revision 1.8  1998/12/02 01:03:39  monaka
 * Major modified for implementing scripting language.
 * But there are a partly connection to user interface.
 *
 * Revision 1.7  1998/11/26 11:14:29  monaka
 *
 * Command evaluator has moved to libMitten/evaluator.c
 * This modify is for Scripting Environment but has not implemented yet.
 *
 * Revision 1.6  1998/11/26 10:19:07  monaka
 *
 * Parser routines has moved to libMitten/parse.c and modified.
 * This modify makes you use multi word string and some escape character on
 * init scripting interpreter.
 * Note that you can't use Mitten language yet. The command syntax is still
 * traditional.
 *
 * Revision 1.5  1997/08/31 14:04:33  night
 * BOOT が HD から行われた時には、自動的に POSIX の root file system を
 * 設定するように処理を変更した。
 *
 * Revision 1.4  1997/05/06 12:44:26  night
 * 入力行をそのまま出力する print 文を削除した。
 *
 * Revision 1.3  1997/04/24 15:37:32  night
 * parse_line() が、文字列のバッファの最後をチェックする値として、
 * 解析後の配列(各コマンドの引数として使用)のサイズを使用していた。
 * そのため、配列のサイズ(10) よりも長い行を入力すると、動作がおかしく
 * なった。
 *
 * Revision 1.2  1996/07/25  16:58:52  night
 * 読み込む行をオーバーフローするときの処理を変更。
 * 以前は、バッファのサイズをオーバーフローする処理に
 * バグがあり、オーバーフローしたことが探知できなかった。
 *
 * Revision 1.1  1996/07/25  16:02:00  night
 * IBM PC 版用への最初の登録
 *
 * Revision 1.3  1996/01/06 12:16:35  night
 * 入力した文字列を配列に変換する処理の変更。
 * (parse_line ())
 *
 * Revision 1.2  1995/12/05 14:34:56  night
 * o デバイスの初期化処理を init_device() 関数に移動する。
 * o (擬似)標準入出力関数への対応。
 *
 * Revision 1.1  1995/10/10  16:21:46  night
 * 最初の登録.
 * システム起動後、プロンプトを出力してコマンド入力 - 実行ができるところ
 * まで出来た。ただし、コマンドは echo だけ。
 *
 *
 */
#include <device.h>
#include <string.h>
#include <unistd.h>
#include <itron/syscall.h>
#include <itron/errno.h>
#include <lowlib.h>
#include "../../servers/keyboard/keyboard.h"
#include "posix_if.h"
#include "init-stdlib.h"
#include "init.h"


/*
 * 入出力を行う。
 */
extern void lowlib_load (B *name);
static void exec(char *);
static void banner(void);
static W read_line_edit(B * line, W length);

/*
 *
 */
int main(int ac, B ** av)
{
    static B line[100];
    UW rootfs;
    ER errno;
    ID myself;

    get_tid(&myself);
    chg_pri(myself, USER_LEVEL);

    init_device();

    banner();

    errno = vsys_inf(1, 0, &rootfs);
    if (errno == E_OK) {
	errno = posix_init(rootfs);
    }

    if (errno != E_OK) {
    	return -1;
    }

    lowlib_load("lowlib.posix");
    strcpy((LOWLIB_DATA)->dpath, "/");
    (LOWLIB_DATA)->dpath_len = 1;

    for (;;) {
	printf ("# ");
	read_line_edit(line, sizeof(line));
	exec(line);
    }

    return(0);
}

static void exec(char *str)
{
    int pid, res;
    int i, flg, argc, len = strlen(str);

    for (i = 0, flg = 0, argc = 0; i < len; i++) {
	if (isspace(str[i])) {
	    flg = 0;
	    continue;
	}

	if (flg == 0) {
	    flg = 1;
	    ++argc;
	}
    }

    if (!argc)	return;

    if ((res = fork()) == 0) {
	char *argv[argc + 1];

	for (i = 0, flg = 0, argc = 0; i < len; i++) {
	    if (isspace(str[i])) {
		str[i] = 0;
		flg = 0;
		continue;
	    }

	    if (flg == 0) {
		flg = 1;
		argv[argc] = &(str[i]);
		++argc;
	    }
	}

	argv[argc] = 0;
	execve(argv[0], argv, NULL);

	_exit(1);
    }
    else if (res == -1) {
	printf("fork error\n");
    }
    else {
	printf("[INIT] waiting pid=%d ...\n", res);
	pid = waitpid(-1, &res, 0);
	printf("[INIT] child process exited pid=%d stat=%d\n", pid, res);
    }
}

static void banner(void)
{
    printf("init version %d.%d\n", MAJOR_VERSION, MINOR_VERSION);
}

static W read_line_edit(B * line, W length)
{
    W i;
    W ch;

    for (i = 0; i < length - 1;) {
	ch = getc(stdin);
	if ((ch == C('m')) || (ch == C('j')))
	    break;
	else if (ch == '\b') {
	    if (i > 0) {
		--i;
		line[i] = 0;
		printf("\b");
	    }
	} else if (isprint(ch)) {
	    line[i++] = ch;
	}
    }
    line[i] = '\0';
    return (i);
}
