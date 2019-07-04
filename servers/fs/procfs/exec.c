/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2003, Tomohide Naniwa

*/


/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/manager/exec.c,v 1.10 2000/05/06 03:52:22 naniwa Exp $ */


/* exec.c --- exec システムコール関連のファイル
 *
 *
 * $Log: exec.c,v $
 * Revision 1.10  2000/05/06 03:52:22  naniwa
 * implement mkdir/rmdir, etc.
 *
 * Revision 1.9  2000/01/08 09:10:55  naniwa
 * minor fix
 *
 * Revision 1.8  1999/11/10 10:48:31  naniwa
 * to implement execve
 *
 * Revision 1.7  1999/03/24 03:54:41  monaka
 * printf() was renamed to printk().
 *
 * Revision 1.6  1998/07/01 13:57:27  night
 * テストのために一時的に exec する実行ファイルの読み込み処理をスキップす
 * るように変更。
 *
 * Revision 1.5  1998/06/11 15:29:57  night
 * alloca を使用しないよう変更。
 *
 * Revision 1.4  1998/02/16 14:20:28  night
 * コメントの変更。
 *
 * Revision 1.3  1998/01/06 16:39:54  night
 * デバッグ用の print 文の追加。
 *
 * Revision 1.2  1997/10/24 13:55:41  night
 * ファイル中の文字コードを SJIS から EUC に変更。
 *
 * Revision 1.1  1997/10/23 14:32:32  night
 * exec システムコール関係の処理の更新
 *
 *
 *
 */

#include <elf.h>
#include <fcntl.h>
#include <string.h>
#include <boot/init.h>
#include <nerve/kcall.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include "../../lib/libserv/libmm.h"
#include "api.h"
#include "procfs/process.h"

static W read_exec_header(vnode_t *ip, Elf32_Addr *entry,
			  Elf32_Phdr *text,
			  Elf32_Phdr *data);
static W load_segment(vnode_t *ip, Elf32_Phdr *segment,
		   ID task);



/* exec_program - 
 *
 */
W exec_program(pm_args_t *req, session_t * session, vnode_t * parent, B * pathname)
{
    vnode_t *ip;
    W error_no;
    struct permission *acc = &(session->permission);
    Elf32_Addr entry;
    ID caller;
    pid_t pid = req->arg4;

    /* 対象となるプログラムファイルをオープンする */
    error_no = vfs_open(parent, pathname, O_RDONLY, 0, acc, &ip);
    if (error_no) {
	return (error_no);
    }

    do {
	Elf32_Phdr text, data;

	/* 実行許可のチェック */
	error_no = vfs_permit(ip, acc, X_OK);
	if (error_no)
	    break;

	error_no = read_exec_header(ip, &entry, &text, &data);
	if (error_no)
	    break;

	/* region の解放 */
	error_no = process_clean(pid);
	if (error_no)
	    break;

	if (process_create(pid, (VP)(text.p_vaddr),
		(size_t)(data.p_vaddr) + data.p_memsz - (UW)(text.p_vaddr),
		(VP)USER_HEAP_MAX_ADDR)) {
	    error_no = ENOMEM;
	    break;
	}

	/* タスクの context.eip を elf_header.e_entry に設定する */
	caller = process_set_context(pid,
		entry,
		(B*)(req->arg2),
		req->arg3);

	if (caller < 0) {
	    error_no = ECONNREFUSED;
	    break;
	}

	/* テキスト領域をメモリに入れる
	 */
	error_no = load_segment(ip, &text, caller);
	if (error_no)
	    break;

	/* データ領域をメモリに入れる
	 */
	error_no = load_segment(ip, &data, caller);

    } while (false);

    vnodes_remove(ip);

    if (error_no) {
	return (error_no);
    }

    kcall->thread_start(caller);
    return E_OK;
}



/* read_exec_header - 実行ファイルのヘッダを読み取る
 *
 *
 */
static W
read_exec_header(vnode_t *ip,
		 Elf32_Addr *entry,
		 Elf32_Phdr *text, Elf32_Phdr *data)
{
    W error_no;
    size_t rlength;
    Elf32_Phdr ph_table[10];
    Elf32_Ehdr elf_header;
    W ph_index;
    copier_t copier = {
	copy_to,
	(char*)&elf_header
    };
    error_no =
	vfs_read(ip, &copier, 0, sizeof(Elf32_Ehdr),
		     &rlength);
    if (error_no) {
	return error_no;
    }

    if (!isValidModule(&elf_header))
	return (ENOEXEC);

    if (sizeof(Elf32_Phdr) != elf_header.e_phentsize) {
	/* プログラムヘッダのサイズが定義と違っている
	 */
	return (ENOEXEC);
    }

    copier.buf = (char*)ph_table;
    error_no =
	vfs_read(ip, &copier, elf_header.e_phoff,
		     elf_header.e_phentsize * elf_header.e_phnum, &rlength);
    if (error_no) {
	return error_no;
    } else if (rlength != elf_header.e_phentsize * elf_header.e_phnum) {
	return (ENOEXEC);
    }

    memset((VP)text, 0, sizeof(Elf32_Phdr));
    memset((VP)data, 0, sizeof(Elf32_Phdr));
    for (ph_index = 0; ph_index < elf_header.e_phnum; ph_index++) {
	/* プログラムヘッダテーブルを順々に見ていき、各セクションのタイプによって
	 * テキスト、データ、BSS の各情報の初期化を行う。
	 */
	if (ph_table[ph_index].p_type == PT_LOAD) {
	    if (ph_table[ph_index].p_flags == (PF_R + PF_X)) {
		/* TEXT */
		if (text->p_type == 0) {
		    *text = ph_table[ph_index];
		}
	    }
	    /* else if (ph_table[ph_index].p_flags == (PF_R + PF_W + PF_X)) */
	    /* データセクションのフラグは PF_X を含まないようだ */
	    else if (ph_table[ph_index].p_flags == (PF_R + PF_W)) {
		/* DATA */
		if (data->p_type == 0) {
		    *data = ph_table[ph_index];
		}
	    }
	}
    }

    *entry = elf_header.e_entry;
    return (EOK);
}



/* 領域をメモリ中にロードする。
 *
 *
 */
static W
load_segment(vnode_t *ip, Elf32_Phdr *segment, ID task)
{
    W error_no;
    size_t read_size;
    copier_t copier = {
	copy_to_user,
	(char*)(segment->p_vaddr),
	task
    };
    error_no = vfs_read(ip, &copier, segment->p_offset, segment->p_filesz,
	    &read_size);
    if (error_no) {
	return error_no;
    }

    return (EOK);
}
