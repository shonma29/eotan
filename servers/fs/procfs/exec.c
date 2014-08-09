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
#include "../../lib/libserv/libmm.h"
#include "fs.h"

static W set_local(ID pid, ID tskid);
static W read_exec_header(struct inode *ip, Elf32_Addr *entry,
			  Elf32_Phdr *text,
			  Elf32_Phdr *data);
static W load_segment(W procid, struct inode *ip, Elf32_Phdr *segment,
		   ID task);



/* exec_program - 
 *
 */
W exec_program(struct posix_request *req, W procid, B * pathname)
{
    struct inode *ip;
    W error_no;
    struct access_info acc;
    Elf32_Addr entry;
    struct proc *procp;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;
    ID caller;

    /* プロセスの情報の取りだし */
    error_no = proc_get_procp(procid, &procp);
    if (error_no) {
	return (error_no);
    }

    /* 対象となるプログラムファイルをオープンする */
    proc_get_uid(procid, &(acc.uid));
    proc_get_gid(procid, &(acc.gid));
    if (pathname[0] == '/') {
	error_no = fs_open_file(pathname, O_RDONLY, 0, &acc, rootfile, &ip);
    } else {
	struct inode *startip;

	error_no = proc_get_cwd(procid, &startip);
	if (error_no) {
	    return (error_no);
	}
	error_no = fs_open_file(pathname, O_RDONLY, 0, &acc, startip, &ip);
    }
    if (error_no) {
	return (error_no);
    }

    do {
	Elf32_Phdr text, data;

	/* 実行許可のチェック */
	error_no = permit(ip, &acc, X_OK);
	if (error_no)
	    break;

	error_no = read_exec_header(ip, &entry, &text, &data);
	if (error_no)
	    break;

	/* region の解放 */
	error_no = process_destroy(procid);
	if (error_no)
	    break;

	if (process_create(procid, (VP)(text.p_vaddr),
		(size_t)(data.p_vaddr) + data.p_memsz - (UW)(text.p_vaddr),
		(VP)USER_HEAP_MAX_ADDR)) {
	    error_no = ENOMEM;
	    break;
	}

	/* タスクの context.eip を elf_header.e_entry に設定する */
	caller = process_set_context(procid,
		entry,
		req->param.par_execve.stackp,
		req->param.par_execve.stsize);

	if (caller < 0) {
	    error_no = ECONNREFUSED;
	    break;
	}

	procp->proc_maintask = caller;

	/* テキスト領域をメモリに入れる
	 */
	error_no = load_segment(procid, ip, &text, caller);
	if (error_no)
	    break;

	/* データ領域をメモリに入れる
	 */
	error_no = load_segment(procid, ip, &data, caller);

    } while (false);

    fs_close_file(ip);

    if (error_no) {
	return (error_no);
    }

    if (procid == INIT_PID)
	set_local(procid, caller);
    else
	error_no = kcall->region_put(caller,
		&(((thread_local_t*)LOCAL_ADDR)->thread_id),
		sizeof(caller), &(caller));

    if (error_no) {
	return (error_no);
    }

    strncpy(procp->proc_name, pathname, PROC_NAME_LEN - 1);
    procp->proc_name[PROC_NAME_LEN - 1] = '\0';

    kcall->thread_start(caller);
    return E_OK;
}



/* read_exec_header - 実行ファイルのヘッダを読み取る
 *
 *
 */
static W
read_exec_header(struct inode *ip,
		 Elf32_Addr *entry,
		 Elf32_Phdr *text, Elf32_Phdr *data)
{
    W error_no;
    W rlength;
    Elf32_Phdr ph_table[10];
    Elf32_Ehdr elf_header;
    W ph_index;

    error_no =
	fs_read_file(ip, 0, (B *) &elf_header, sizeof(Elf32_Ehdr),
		     &rlength);
    if (error_no) {
	return (error_no);
    }

    if (!isValidModule(&elf_header))
	return (ENOEXEC);

    if (sizeof(Elf32_Phdr) != elf_header.e_phentsize) {
	/* プログラムヘッダのサイズが定義と違っている
	 */
	return (ENOEXEC);
    }

    error_no =
	fs_read_file(ip, elf_header.e_phoff, (B *) ph_table,
		     elf_header.e_phentsize * elf_header.e_phnum, &rlength);
    if (error_no) {
	return (error_no);
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
load_segment(W procid, struct inode *ip, Elf32_Phdr *segment, ID task)
{
    W error_no;
    W rest_length;
    W offset;
    W read_size;
    W vaddr;
    static B buf[PAGE_SIZE];
    UW start, size;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    start = pageRoundDown(segment->p_vaddr);
    size =
	pageRoundUp(segment->p_memsz +
		(segment->p_vaddr - pageRoundDown(segment->p_vaddr)));

    for (rest_length = segment->p_filesz, offset = segment->p_offset, vaddr =
	 segment->p_vaddr; rest_length > 0;
	 rest_length -= PAGE_SIZE, vaddr += PAGE_SIZE, offset += read_size) {
	error_no =
	    fs_read_file(ip, offset, buf,
			 (PAGE_SIZE <
			  rest_length) ? PAGE_SIZE : rest_length,
			 &read_size);
	if (error_no) {
	    return (ENOMEM);
	}

	error_no = kcall->region_put(task, (B *) vaddr, read_size, buf);
	if (error_no) {
	    return (ENOMEM);
	}
    }

    return (EOK);
}

static W set_local(ID pid, ID tskid)
{
    W error_no;
    thread_local_t local_data;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    error_no = vmap(pid, (thread_local_t*)LOCAL_ADDR, sizeof(thread_local_t),
    		true);
    if (error_no)
	return error_no;

    memset(&local_data, 0, sizeof(local_data));
    local_data.thread_id = tskid;
    local_data.process_id = pid;
    strcpy((B*)local_data.cwd, "/");
    local_data.cwd_length = 1;

    error_no = kcall->region_put(tskid, (thread_local_t*)LOCAL_ADDR,
		     sizeof(thread_local_t), &local_data);
    if (error_no)
	return error_no;
    return (EOK);
}

