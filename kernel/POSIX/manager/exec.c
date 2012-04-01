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


#include "posix.h"

static W read_exec_header(struct inode *ip, struct ELFheader *elfp,
			  struct ELF_Pheader *text,
			  struct ELF_Pheader *data);
static W load_text(W procid, struct inode *ip, struct ELF_Pheader *text,
		   ID task);
static W load_data(W procid, struct inode *ip, struct ELF_Pheader *data,
		   ID task);



/* exec_program - 
 *
 */
W exec_program(struct posix_request *req, W procid, B * pathname)
{
    struct inode *ip;
    W errno;
    struct access_info acc;
    struct ELFheader elf_header;
    struct ELF_Pheader text, data;
    ID main_task;
#if 0
    ID signal_task;
#endif
    struct proc *procp;

#ifdef notdef
    printk("[PM] exec_program: path = \"%s\"\n", pathname);	/* */
#endif

    /* プロセスの情報の取りだし */
    errno = proc_get_procp(procid, &procp);
    if (errno) {
	return (errno);
    }

    /* 対象となるプログラムファイルをオープンする */
    proc_get_euid(procid, &(acc.uid));
    proc_get_egid(procid, &(acc.gid));
    if (pathname[0] == '/') {
#ifdef notdef
	dbg_printf("[PM] exec_program: call fs_open_file ()\n");	/* */
#endif
	errno = fs_open_file(pathname, O_RDONLY, 0, &acc, rootfile, &ip);
    } else {
	struct inode *startip;

	errno = proc_get_cwd(procid, &startip);
	if (errno) {
	    return (errno);
	}
	errno = fs_open_file(pathname, O_RDONLY, 0, &acc, startip, &ip);
    }
    if (errno) {
#ifdef EXEC_DEBUG
	dbg_printf("[PM] Cannot open file. -> return from exec_program().\n");
#endif
	return (errno);
    }

    /* 実行許可のチェック */
    errno = permit(ip, &acc, X_BIT);
    if (errno) {
#ifdef EXEC_DEBUG
	dbg_printf("[PM] Permission denied. -> return from exec_program().\n");
#endif
	return (errno);
    }
#ifdef notdef
    dbg_printf("[PM] read exec header\n");
#endif
    errno = read_exec_header(ip, &elf_header, &text, &data);
    if (errno) {
	fs_close_file(ip);
	return (errno);
    }
#ifdef notdef
    dbg_printf("[PM] destroy\n");	/* */
#endif
    /* 呼び出し元プロセスのすべての(仮想)メモリを解放する
     */
    errno = proc_destroy_memory(procid);
    if (errno) {
	fs_close_file(ip);
	return (errno);
    }
#ifdef notdef
    dbg_printf("[PM] vdel_reg\n");	/* */
#endif
    /* region の解放 */
    main_task = procp->proc_maintask;
    vdel_reg(main_task, TEXT_REGION);	/* text */
    vdel_reg(main_task, DATA_REGION);	/* data+bss */
    vdel_reg(main_task, HEAP_REGION);	/* heap */
#ifdef notdef
    /* stack 領域は fork で生成した region を使い続ける */
    vdel_reg(main_task, STACK_REGION);	/* stack */
#endif

#if 0 /* never use */
    /* 新しいタスクを生成する
     */
    /* 既存のタスクのイメージを変更することとし、タスクの生成は止める */
    errno = proc_renew_task(procid, req->param.par_execve.start_main,
			    req->param.par_execve.start_signal,
			    &main_task, &signal_task);
    if (errno) {
	fs_close_file(ip);
	return (errno);
    }
#endif
#ifdef notdef
    dbg_printf("[PM] setup vm proc\n");
#endif
    /* 仮想空間の生成 */
    errno = setup_vm_proc(procp);

#ifdef notdef
    dbg_printf("[PM] load text\n");
#endif
    /* テキスト領域をメモリに入れる
     */
    errno = load_text(procid, ip, &text, req->caller);
    if (errno) {
	fs_close_file(ip);
	return (errno);
    }

    /* データ領域をメモリに入れる
     */
#ifdef notdef
    dbg_printf("[PM] load data\n");
#endif
    errno = load_data(procid, ip, &data, req->caller);
    if (errno) {
	fs_close_file(ip);
	return (errno);
    }

    /* 残りの region の作成 */
    errno = vcre_reg(req->caller, HEAP_REGION,
		     (VP) VADDR_HEAP, 0, STD_HEAP_SIZE,
	     VM_READ | VM_WRITE | VM_USER, NULL);	/* heap */
#ifdef DEBUG
    if (errno) {
      dbg_printf("[EXEC]: vcre_reg return %d\n", errno);
    }
    {
      T_REGION reg;

      errno = vsts_reg(req->caller, HEAP_REGION, (VP) & reg);
      dbg_printf("[EXEC] err = %d sa %x, min %x, max %x\n",
		 errno, reg.start_addr, reg.min_size, reg.max_size);
    }
#endif

#if 0
    /* stack の region は fork から呼ばれる vcpy_stk で実行される */
    vcre_reg(req->caller, STACK_REGION,
	     (VP) VADDR_STACK_HEAD, STD_STACK_SIZE, STD_STACK_SIZE,
	     VM_READ | VM_WRITE | VM_USER, NULL);	/* stack */
#endif

#if 0
    {
	int stsize = req->param.par_execve.stsize, i;
	char buf[stsize];
	int *bufp;

	errno = vget_reg(req->caller, req->param.par_execve.stackp,
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
    errno = vset_ctx(req->caller, elf_header.e_entry,
		     req->param.par_execve.stackp,
		     req->param.par_execve.stsize);

#ifdef notdef
    dbg_printf("[PM] close_file\n");
#endif
    fs_close_file(ip);

    strncpy(procp->proc_name, pathname, PROC_NAME_LEN);

#ifdef notdef
    dbg_printf("[PM] exec return %d\n", errno);
#endif
    if (errno)
	return errno;

    return (EP_OK);
}



/* read_exec_header - 実行ファイルのヘッダを読み取る
 *
 *
 */
static W
read_exec_header(struct inode *ip,
		 struct ELFheader *elfp,
		 struct ELF_Pheader *text, struct ELF_Pheader *data)
{
    W errno;
    W rlength;
#ifdef USE_ALLOCA
    struct ELF_Pheader *ph_table;
#else
    struct ELF_Pheader ph_table[10];
#endif
    W ph_index;


    errno =
	fs_read_file(ip, 0, (B *) elfp, sizeof(struct ELFheader),
		     &rlength);
    if (errno) {
	return (errno);
    }

    /* マジックナンバのチェック
     */
    if ((elfp->e_ident[0] != 0x7f) ||
	(elfp->e_ident[1] != 'E') ||
	(elfp->e_ident[2] != 'L') || (elfp->e_ident[3] != 'F')) {
	/* ELF フォーマットのファイルではなかった
	 */
	return (EP_NOEXEC);
    }

    if (elfp->e_type != ET_EXEC) {
	/* 実行ファイルではなかった
	 */
	return (EP_NOEXEC);
    }

    if (elfp->e_machine != EM_386) {
	/* ELF ファイルの対応マシン種類が違った
	 */
	return (EP_NOEXEC);
    }

    if (elfp->e_version < 1) {
	/* ELF ファイルのバージョンが不正
	 */
	return (EP_NOEXEC);
    }

    if (sizeof(struct ELF_Pheader) != elfp->e_phentsize) {
	/* プログラムヘッダのサイズが定義と違っている
	 */
	return (EP_NOEXEC);
    }
#ifdef USE_ALLOCA
    ph_table = alloca(elfp->e_phentsize * elfp->e_phnum);
    if (ph_table == NULL) {
	/* メモリが取得できなかった
	 */
	return (EP_NOMEM);
    }
#endif

    errno =
	fs_read_file(ip, elfp->e_phoff, (B *) ph_table,
		     elfp->e_phentsize * elfp->e_phnum, &rlength);
    if (errno) {
	return (errno);
    } else if (rlength != elfp->e_phentsize * elfp->e_phnum) {
	return (EP_NOEXEC);
    }

    bzero((VP) text, sizeof(struct ELF_Pheader));
    bzero((VP) data, sizeof(struct ELF_Pheader));
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

    return (EP_OK);
}



/* テキスト領域をメモリ中にロードする。
 *
 *
 */
static W
load_text(W procid, struct inode *ip, struct ELF_Pheader *text, ID task)
{
    W errno;
    W rest_length;
    W offset;
    W read_size;
    W vaddr;
    static B buf[PAGE_SIZE];
    UW start, size;

    start = CUTDOWN(text->p_vaddr, PAGE_SIZE);
    size =
	ROUNDUP(text->p_memsz +
		(text->p_vaddr - CUTDOWN(text->p_vaddr, PAGE_SIZE)),
		PAGE_SIZE);
    /* text region の設定 */
    vcre_reg(task, TEXT_REGION, (VP) start, size, size,
	     VM_READ | VM_EXEC | VM_USER, NULL);

    errno = alloc_memory(procid, start, size, VM_READ | VM_EXEC);
    if (errno) {
#ifdef EXEC_DEBUG
	dbg_printf("ERROR: alloc memory\n");
#endif
	return (errno);
    }

    for (rest_length = text->p_filesz, offset = text->p_offset, vaddr =
	 text->p_vaddr; rest_length > 0;
	 rest_length -= PAGE_SIZE, vaddr += PAGE_SIZE, offset += read_size) {
	errno =
	    fs_read_file(ip, offset, buf,
			 (PAGE_SIZE <
			  rest_length) ? PAGE_SIZE : rest_length,
			 &read_size);
	if (errno) {
#ifdef EXEC_DEBUG
	    dbg_printf("ERROR: fs_read_file\n");
#endif
	    return (EP_NOMEM);
	}

	errno = vput_reg(task, (B *) vaddr, read_size, buf);
	if (errno) {
#ifdef EXEC_DEBUG
	    dbg_printf("ERROR: vput_reg %d %d %x %d %x\n", errno,
		   task, vaddr, read_size, buf);
#endif
	    return (EP_NOMEM);
	}
    }

    return (EP_OK);
}



/* データ領域をメモリ中にロードする。
 *
 *
 */
static W
load_data(W procid, struct inode *ip, struct ELF_Pheader *data, ID task)
{
    W errno;
    W rest_length;
    W offset;
    W read_size;
    W vaddr;
    static B buf[PAGE_SIZE];
    UW start, size;

    start = CUTDOWN(data->p_vaddr, PAGE_SIZE);
    size =
	ROUNDUP(data->p_memsz +
		(data->p_vaddr - CUTDOWN(data->p_vaddr, PAGE_SIZE)),
		PAGE_SIZE);
    /* data+bss region の設定 */
    vcre_reg(task, DATA_REGION, (VP) start, size, size,
	     VM_READ | VM_WRITE | VM_USER, NULL);	/* data+bss */
    errno = alloc_memory(procid, start, size, VM_READ | VM_WRITE);
    if (errno) {
#ifdef EXEC_DEBUG
	dbg_printf("ERROR: alloc memory\n");
#endif
	return (errno);
    }

    for (rest_length = data->p_filesz, offset = data->p_offset, vaddr =
	 data->p_vaddr; rest_length > 0;
	 rest_length -= PAGE_SIZE, vaddr += PAGE_SIZE, offset += read_size) {
	errno =
	    fs_read_file(ip, offset, buf,
			 (PAGE_SIZE <
			  rest_length) ? PAGE_SIZE : rest_length,
			 &read_size);
	if (errno) {
#ifdef EXEC_DEBUG
	    dbg_printf("ERROR: fs_read_file\n");
#endif
	    return (EP_NOMEM);
	}

	errno = vput_reg(task, (B *) vaddr, read_size, buf);
	if (errno) {
#ifdef EXEC_DEBUG
	    dbg_printf("ERROR: vput_reg\n");
#endif
	    return (EP_NOMEM);
	}
    }

    return (EP_OK);
}
