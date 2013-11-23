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
#include <kcall.h>
#include <string.h>
#include <vm.h>
#include <mm/segment.h>
#include <boot/init.h>
#include <mpu/config.h>
#include <mpu/memory.h>
#include "../../lib/libserv/libmm.h"
#include "fs.h"

static W read_exec_header(struct inode *ip, Elf32_Ehdr *elfp,
			  Elf32_Phdr *text,
			  Elf32_Phdr *data);
static W load_text(W procid, struct inode *ip, Elf32_Phdr *text,
		   ID task);
static W load_data(W procid, struct inode *ip, Elf32_Phdr *data,
		   ID task);



/* exec_program - 
 *
 */
W exec_program(struct posix_request *req, W procid, B * pathname)
{
    struct inode *ip;
    W error_no;
    struct access_info acc;
    Elf32_Ehdr elf_header;
    Elf32_Phdr text, data;
    ID main_task;
    struct proc *procp;
#ifdef notdef
    printk("[PM] exec_program: path = \"%s\"\n", pathname);	/* */
#endif

    /* プロセスの情報の取りだし */
    error_no = proc_get_procp(procid, &procp);
    if (error_no) {
	return (error_no);
    }

    /* 対象となるプログラムファイルをオープンする */
    proc_get_euid(procid, &(acc.uid));
    proc_get_egid(procid, &(acc.gid));
    if (pathname[0] == '/') {
#ifdef notdef
	dbg_printf("[PM] exec_program: call fs_open_file ()\n");	/* */
#endif
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
#ifdef EXEC_DEBUG
	dbg_printf("[PM] Cannot open file. -> return from exec_program().\n");
#endif
	return (error_no);
    }

    /* 実行許可のチェック */
    error_no = permit(ip, &acc, X_OK);
    if (error_no) {
#ifdef EXEC_DEBUG
	dbg_printf("[PM] Permission denied. -> return from exec_program().\n");
#endif
	return (error_no);
    }
#ifdef notdef
    dbg_printf("[PM] read exec header\n");
#endif
    error_no = read_exec_header(ip, &elf_header, &text, &data);
    if (error_no) {
	fs_close_file(ip);
	return (error_no);
    }

#ifdef notdef
    dbg_printf("[PM] vdel_reg\n");	/* */
#endif
    /* region の解放 */
    main_task = procp->proc_maintask;
    process_destroy(main_task);

    /* テキスト領域をメモリに入れる
     */
#ifdef notdef
    dbg_printf("[PM] load text\n");
#endif
    error_no = load_text(procid, ip, &text, req->caller);
    if (error_no) {
	fs_close_file(ip);
	return (error_no);
    }

    /* データ領域をメモリに入れる
     */
#ifdef notdef
    dbg_printf("[PM] load data\n");
#endif
    error_no = load_data(procid, ip, &data, req->caller);
    if (error_no) {
	fs_close_file(ip);
	return (error_no);
    }

    /* 残りの region の作成 */
    error_no = process_create(req->caller, seg_heap,
		     (VP) VADDR_HEAP, 0, STD_HEAP_SIZE);	/* heap */
#ifdef DEBUG
    if (error_no) {
      dbg_printf("[EXEC]: vcre_reg return %d\n", error_no);
    }
    {
      mm_segment_t reg;

      error_no = vsts_reg(req->caller, seg_heap, (VP) & reg);
      dbg_printf("[EXEC] err = %d sa %x, min %x, max %x\n",
		 error_no, reg.addr, reg.len, reg.max);
    }
#endif
#if 0
    {
	int stsize = req->param.par_execve.stsize, i;
	char buf[stsize];
	int *bufp;

	error_no = vget_reg(req->caller, req->param.par_execve.stackp,
			 stsize, buf);
	for (bufp = (int *) buf; *bufp != 0; ++bufp) {
	    dbg_printf("%x %s\n", *bufp, &buf[*bufp]);
	}
	for (++bufp; *bufp != 0; ++bufp) {
	    dbg_printf("%x %s\n", *bufp, &buf[*bufp]);
	}
    }
#endif

#ifdef notdef
    dbg_printf("[PM] vset_ctx entry %x stackp %x stsize %d\n",
	       elf_header.e_entry,
	       req->param.par_execve.stackp,
	       req->param.par_execve.stsize);
#endif
    /* タスクの context.eip を elf_header.e_entry に設定する */
    error_no = process_set_context(procid? req->caller:(INIT_THREAD_ID_FLAG | req->caller),
		     elf_header.e_entry,
		     req->param.par_execve.stackp,
		     req->param.par_execve.stsize);

#ifdef notdef
    dbg_printf("[PM] close_file\n");
#endif
    fs_close_file(ip);

    strncpy(procp->proc_name, pathname, PROC_NAME_LEN - 1);
    procp->proc_name[PROC_NAME_LEN - 1] = '\0';

#ifdef notdef
    dbg_printf("[PM] exec return %d\n", error_no);
#endif
    if (error_no)
	return error_no;

    return (EOK);
}



/* read_exec_header - 実行ファイルのヘッダを読み取る
 *
 *
 */
static W
read_exec_header(struct inode *ip,
		 Elf32_Ehdr *elfp,
		 Elf32_Phdr *text, Elf32_Phdr *data)
{
    W error_no;
    W rlength;
    Elf32_Phdr ph_table[10];
    W ph_index;


    error_no =
	fs_read_file(ip, 0, (B *) elfp, sizeof(Elf32_Ehdr),
		     &rlength);
    if (error_no) {
	return (error_no);
    }

    /* マジックナンバのチェック
     */
    if ((elfp->e_ident[0] != 0x7f) ||
	(elfp->e_ident[1] != 'E') ||
	(elfp->e_ident[2] != 'L') || (elfp->e_ident[3] != 'F')) {
	/* ELF フォーマットのファイルではなかった
	 */
	return (ENOEXEC);
    }

    if (elfp->e_type != ET_EXEC) {
	/* 実行ファイルではなかった
	 */
	return (ENOEXEC);
    }

    if (elfp->e_machine != EM_386) {
	/* ELF ファイルの対応マシン種類が違った
	 */
	return (ENOEXEC);
    }

    if (elfp->e_version < 1) {
	/* ELF ファイルのバージョンが不正
	 */
	return (ENOEXEC);
    }

    if (sizeof(Elf32_Phdr) != elfp->e_phentsize) {
	/* プログラムヘッダのサイズが定義と違っている
	 */
	return (ENOEXEC);
    }

    error_no =
	fs_read_file(ip, elfp->e_phoff, (B *) ph_table,
		     elfp->e_phentsize * elfp->e_phnum, &rlength);
    if (error_no) {
	return (error_no);
    } else if (rlength != elfp->e_phentsize * elfp->e_phnum) {
	return (ENOEXEC);
    }

    memset((VP)text, 0, sizeof(Elf32_Phdr));
    memset((VP)data, 0, sizeof(Elf32_Phdr));
    for (ph_index = 0; ph_index < elfp->e_phnum; ph_index++) {
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

    return (EOK);
}



/* テキスト領域をメモリ中にロードする。
 *
 *
 */
static W
load_text(W procid, struct inode *ip, Elf32_Phdr *text, ID task)
{
    W error_no;
    W rest_length;
    W offset;
    W read_size;
    W vaddr;
    static B buf[PAGE_SIZE];
    UW start, size;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    start = pageRoundDown(text->p_vaddr);
    size =
	pageRoundUp(text->p_memsz +
		(text->p_vaddr - pageRoundDown(text->p_vaddr)));
    /* text region の設定 */
    process_create(task, seg_code, (VP) start, size, size);

    error_no = alloc_memory(procid, start, size, VM_READ | VM_EXEC);
    if (error_no) {
#ifdef EXEC_DEBUG
	dbg_printf("ERROR: alloc memory\n");
#endif
	return (error_no);
    }

    for (rest_length = text->p_filesz, offset = text->p_offset, vaddr =
	 text->p_vaddr; rest_length > 0;
	 rest_length -= PAGE_SIZE, vaddr += PAGE_SIZE, offset += read_size) {
	error_no =
	    fs_read_file(ip, offset, buf,
			 (PAGE_SIZE <
			  rest_length) ? PAGE_SIZE : rest_length,
			 &read_size);
	if (error_no) {
#ifdef EXEC_DEBUG
	    dbg_printf("ERROR: fs_read_file\n");
#endif
	    return (ENOMEM);
	}

	error_no = kcall->region_put(task, (B *) vaddr, read_size, buf);
	if (error_no) {
#ifdef EXEC_DEBUG
	    dbg_printf("ERROR: vput_reg %d %d %x %d %x\n", error_no,
		   task, vaddr, read_size, buf);
#endif
	    return (ENOMEM);
	}
    }

    return (EOK);
}



/* データ領域をメモリ中にロードする。
 *
 *
 */
static W
load_data(W procid, struct inode *ip, Elf32_Phdr *data, ID task)
{
    W error_no;
    W rest_length;
    W offset;
    W read_size;
    W vaddr;
    static B buf[PAGE_SIZE];
    UW start, size;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    start = pageRoundDown(data->p_vaddr);
    size =
	pageRoundUp(data->p_memsz +
		(data->p_vaddr - pageRoundDown(data->p_vaddr)));
    /* data+bss region の設定 */
    process_create(task, seg_data, (VP) start, size, size);	/* data+bss */
    error_no = alloc_memory(procid, start, size, VM_READ | VM_WRITE);
    if (error_no) {
#ifdef EXEC_DEBUG
	dbg_printf("ERROR: alloc memory\n");
#endif
	return (error_no);
    }

    for (rest_length = data->p_filesz, offset = data->p_offset, vaddr =
	 data->p_vaddr; rest_length > 0;
	 rest_length -= PAGE_SIZE, vaddr += PAGE_SIZE, offset += read_size) {
	error_no =
	    fs_read_file(ip, offset, buf,
			 (PAGE_SIZE <
			  rest_length) ? PAGE_SIZE : rest_length,
			 &read_size);
	if (error_no) {
#ifdef EXEC_DEBUG
	    dbg_printf("ERROR: fs_read_file\n");
#endif
	    return (ENOMEM);
	}

	error_no = kcall->region_put(task, (B *) vaddr, read_size, buf);
	if (error_no) {
#ifdef EXEC_DEBUG
	    dbg_printf("ERROR: vput_reg\n");
#endif
	    return (ENOMEM);
	}
    }

    return (EOK);
}
