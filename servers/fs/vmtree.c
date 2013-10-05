/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/


/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/manager/vmtree.c,v 1.16 2000/01/26 08:24:36 naniwa Exp $ */


/* vmtree.c - 仮想メモリツリーの管理
 *
 * $Log: vmtree.c,v $
 * Revision 1.16  2000/01/26 08:24:36  naniwa
 * to prevent memory leak
 *
 * Revision 1.15  2000/01/15 15:29:31  naniwa
 * minor fix
 *
 * Revision 1.14  2000/01/08 09:10:27  naniwa
 * fixed initialization of variables, etc
 *
 * Revision 1.13  1999/12/26 11:17:40  naniwa
 * add debug write
 *
 * Revision 1.12  1999/11/10 10:48:35  naniwa
 * to implement execve
 *
 * Revision 1.11  1999/07/26 08:56:26  naniwa
 * minor fix
 *
 * Revision 1.10  1999/07/23 14:39:04  naniwa
 * modified to implement exit
 *
 * Revision 1.9  1999/03/24 03:54:47  monaka
 * printf() was renamed to printk().
 *
 * Revision 1.8  1999/03/02 15:30:58  night
 * 田畑@KMC (yusuke@kmc.kyoto-u.ac.jp) のアドバイスによる修正。
 * --------------------
 * それから別のバグですが、
 * POSIX/manager/vmtree.cの grow_vm() の最初で pageentを
 * 求めるときに
 * pageent = addr % (MAX_PAGE_ENTRY * PAGE_SIZE) ;
 * となっていますが、ここは
 * pageent = (addr / PAGE_SIZE ) % MAX_PAGE_ENTRY ;
 * とか書くべきだと思います。
 * --------------------
 *
 * Revision 1.7  1998/07/01 14:03:10  night
 * デバッグ用のデータ出力処理を #ifdef notdef 〜 #endif で囲んだ。
 *
 * Revision 1.6  1998/02/25 12:53:07  night
 * システムコール vmap_reg の引数追加(アクセス権限を指定する引数を追加)に
 * 伴った修正。
 * アクセス権としては、ACC_USER (ユーザが読み書きできる) を指定した。
 *
 * Revision 1.5  1998/02/24 14:21:42  night
 * プロセスの間の仮想ページのデータのコピーする処理を
 * 正しく動くように修正。
 *
 * Revision 1.4  1998/02/23 14:50:51  night
 * setup_vmtree 関数をきちんと動くようにした。
 *
 * Revision 1.3  1998/02/16 14:28:31  night
 * setup_vmtree() の追加。
 * この関数は、引数で指定した ITRON タスクから仮想メモリテーブル
 * vmtree を生成する。
 *
 * Revision 1.2  1997/10/24 13:56:16  night
 * ファイル中の文字コードを SJIS から EUC に変更。
 *
 * Revision 1.1  1997/10/23 14:32:33  night
 * exec システムコール関係の処理の更新
 *
 *
 */

#include <kcall.h>
#include <string.h>
#include <mpu/config.h>
#include "fs.h"
#include "../../kernel/mpu/mpu.h"

#define VM_DEFAULT_ACCESS (VM_READ | VM_WRITE | VM_USER | VM_SHARE)
#define VM_DEFAULT_START (LOW_USER_ADDR)
#define VM_DEFAULT_SIZE (USER_MEM_SIZE)



/* create_vm_tree - 新しくプロセスの仮想メモリツリーを作成する
 *
 */
W create_vm_tree(struct proc *proc)
{
    struct vm_tree *treep;
    W i;

    treep = (struct vm_tree *) malloc(sizeof(struct vm_tree));
    if (treep == NULL) {
	return (ENOMEM);
    }

    for (i = 0; i < MAX_DIR_ENTRY; i++) {
	treep->directory_table[i] = NULL;
    }

    proc->vm_tree = treep;

    return (EOK);
}



/* grow_vm - 新しく仮想ページに物理メモリを割り当てる
 *
 */
W grow_vm(struct proc * procp, UW addr, UW access)
{
    UW dirent;
    UW pageent;
    struct vm_directory *vmdir;
    struct vm_page *vmpage;
    ER error_no;
    struct vm_tree *treep;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    treep = procp->vm_tree;
    dirent = addr / (MAX_PAGE_ENTRY * PAGE_SIZE);	/* directory table のインデックス */
    pageent = (addr >> PAGE_SHIFT) % MAX_PAGE_ENTRY;	/* page table のインデックス */

    vmdir = treep->directory_table[dirent];
    if (vmdir == NULL) {
	vmdir =
	    (struct vm_directory *) alloc_vm_directory(treep,
						       dirent *
						       MAX_PAGE_ENTRY *
						       PAGE_SIZE);
	if (vmdir == NULL) {
	    return (ENOMEM);
	}
	treep->directory_table[dirent] = vmdir;
    }

    vmpage = vmdir->page_table[pageent];
    if (vmpage == NULL) {
	vmpage = (struct vm_page *) alloc_vm_page(treep, vmdir,
						  addr,
						  access);
	if (vmpage == NULL) {
	    return (ENOMEM);
	}
	vmdir->page_table[pageent] = vmpage;
    }

    /* 仮想メモリ領域に物理メモリを割り付ける
     */
    error_no = kcall->region_map(procp->proc_maintask, (VP) addr, PAGE_SIZE, ACC_USER);
    if (error_no) {
	return (EPERM);
    }

    return (EOK);
}


/* shorten_vm - 仮想ページを解放する
 *
 */
W shorten_vm(struct proc * procp, UW addr)
{
    UW dirent;
    UW pageent;
    struct vm_directory *vmdir;
    struct vm_page *vmpage;
    ER error_no;
    struct vm_tree *treep;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    treep = procp->vm_tree;
    dirent = addr / (MAX_PAGE_ENTRY * PAGE_SIZE);
    /* page table のインデックス */
    pageent = (addr >> PAGE_SHIFT) % MAX_PAGE_ENTRY;

    vmdir = treep->directory_table[dirent];
    if (vmdir == NULL) {
	return (EINVAL);
    }
    vmpage = vmdir->page_table[pageent];
    if (vmpage == NULL) {
	return (EINVAL);
    }
    error_no = kcall->region_unmap(procp->proc_maintask, (VP) (vmpage->addr), PAGE_SIZE);
    if (error_no) {
	return (EINVAL);
    }

    /* 使わなくなった page table のエントリーは free する */
    vmdir->page_table[pageent] = NULL;
    free(vmpage);

    return (EOK);
}



/* duplicate_tree - vm_tree の情報をコピーする
 *
 *
 */
W duplicate_tree(struct proc * source_proc, struct proc * dest_proc)
{
    W dir_index;
    W page_index;
    struct vm_directory *dirp, *dest_dirp;
    struct vm_page *pagep, *dest_pagep;
    ER error_no;
    struct vm_tree *source;
    struct vm_tree *destination;
    static B page_buf[PAGE_SIZE];
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    source = source_proc->vm_tree;
    destination = dest_proc->vm_tree;

#ifdef VMDEBUG
    printk
	("duplicate_tree: %s, %d, source process ID = %d, parent = 0x%x\n",
	 __FILE__, __LINE__, source_proc->proc_pid, source_proc);	/* */
    printk("source_proc->vmtree = 0x%x\n", source);	/* */
#endif


    /* プロセスのもつページディレクトリ情報をひとつひとつ調べていく。
     * もし、ディレクトリのエントリのうち使っているものがあれば、
     * その内容をコピーし、そのディレクトリ中のページ(複数)の内容も
     * チェックする。
     */
    for (dir_index = 0; dir_index < (MAX_DIR_ENTRY / 2); dir_index++) {
	dirp = source->directory_table[dir_index];
#ifdef VMDEBUG
	if ((dir_index == 0) && (dirp == 0)) {
	    printk("%s: %d: ?? directory 0 entry is clear.\n", __FILE__, __LINE__);	/* */
	}
#endif

	if (dirp != NULL) {
	    /* コピー元のページディレクトリ内に情報があった。
	     */

#ifdef notdef
	    printk("duplicate_tree: direntry is %d\n", dir_index);	/* */
#endif

	    dest_dirp = destination->directory_table[dir_index];
	    if (dest_dirp == NULL) {
		dest_dirp =
		    (struct vm_directory *) alloc_vm_directory(destination,
							       dir_index *
							       MAX_PAGE_ENTRY
							       *
							       PAGE_SIZE);
		if (dest_dirp == NULL) {
#ifdef VMDEBUG
		    printk("duplicate_tree: empty dest_dirp %d\n",
			   dir_index);
#endif
		    return (ENOMEM);
		}
		destination->directory_table[dir_index] = dest_dirp;
	    } else {
		/* すでに受けがわページディレクトリが使用されている
		 */
#ifdef VMDEBUG
		printk("duplicate_tree: non null dest_dirp\n");
#endif
		return (EINVAL);
	    }


	    /* ディレクトリ中の各ページチェックし、もしコピー元のページが実メモリに
	     * マッピングされていた場合には、内容をコピー先にコピーする
	     */
	    for (page_index = 0; page_index < MAX_PAGE_ENTRY; page_index++) {
		pagep = dirp->page_table[page_index];

#ifdef VMDEBUG
		printk("duplicate_tree: page is %d, pagep = 0x%x\n",
		       page_index, pagep);	/* */
#endif

		if (pagep) {
		    dest_pagep = dest_dirp->page_table[page_index];
		    if (dest_pagep == NULL) {
			/* 新しいページ情報をアロケート */
			dest_pagep =
			    (struct vm_page *) alloc_vm_page(destination,
							     dest_dirp, 0,
							     VM_DEFAULT_ACCESS);
			if (dest_pagep == NULL) {
#ifdef VMDEBUG
			    printk("duplicate_tree: empty pagep\n");
#endif
			    return (ENOMEM);
			}
#ifdef VMDEBUG
			printk("allocate new page information: 0x%x\n", dest_pagep);	/* */
#endif

			dest_dirp->page_table[page_index] = dest_pagep;

#ifdef VMDEBUG
			/* プロセスの仮想アドレスに実ページをマップする
			 */
			printk("duplicate_tree: vmap_reg (0x%x, %d)\n",
			       (VP) (dir_index * MAX_PAGE_ENTRY *
				     PAGE_SIZE) + (page_index * PAGE_SIZE), PAGE_SIZE);	/* */
#endif

			dest_pagep->addr =
			    (dir_index * MAX_PAGE_ENTRY * PAGE_SIZE) +
			    (page_index << PAGE_SHIFT);
			error_no =
			    kcall->region_map(dest_proc->proc_maintask,
				     (VP) dest_pagep->addr, PAGE_SIZE,
				     ACC_USER);
			if (error_no) {
#ifdef VMDEBUG
			    printk("cannot vmap_reg: errno = %d\n", error_no);
#endif
			    return (error_no);
			}
		    } else {
			/* すでに受けがわページが使用されている
			 */
#ifdef VMDEBUG
			printk("duplicate_tree: non empty page\n");
#endif
			return (EINVAL);
		    }

		    /* 送り側プロセスのメモリ中の情報を取り出す
		     */
		    error_no = kcall->region_get(source_proc->proc_maintask,
				     (VP) dest_pagep->addr,
				     PAGE_SIZE, (VP) page_buf);
		    if (error_no) {
#ifdef VMDEBUG
			printk("vget_reg: errno = %d\n", error_no);
			printk
			    ("          task = %d, addr = %x, buf = %x\n",
			     source_proc->proc_maintask, dest_pagep->addr,
			     (VP) page_buf);
#endif
			return (error_no);
		    }
#ifdef notdef
		    if (
			((dir_index * MAX_PAGE_ENTRY * PAGE_SIZE) +
			 (page_index << PAGE_SHIFT)) < PAGE_DIR_ADDR) {
			W i, j;

			printk
			    ("vget_reg (pid = %d, addr = 0x%x, size = 0x%x, buf = 0x%x)\n",
			     source_proc->proc_maintask,
			     (VP) ((dir_index * MAX_PAGE_ENTRY * PAGE_SIZE)
				   + (page_index * PAGE_SIZE)), PAGE_SIZE,
			     (VP) page_buf);

			for (i = 0; i < 64;) {
			    for (j = 0; j < 8; j++, i++) {
				if (page_buf[i] < 0xA) {
				    printk("0x0%x ", page_buf[i]);
				} else {
				    printk("0x%x ", page_buf[i]);
				}
			    }
			    printk("\n");
			}
		    }
#endif

		    /* 受け側プロセスのメモリに取り出した情報を送る
		     */
		    error_no = kcall->region_put(dest_proc->proc_maintask,
				     (VP) dest_pagep->addr,
				     PAGE_SIZE, (VP) page_buf);
		    if (error_no) {
#ifdef VMDEBUG
			printk("vput_reg: errno = %d\n", error_no);
#endif
			return (error_no);
		    }
		}
	    }			/* ディレクトリの各ページのチェックのループの最後 */
	}
    }				/* プロセスの各ページディレクトリのチェックのループの最後 */


    return (EOK);
}




struct vm_page *alloc_vm_page(struct vm_tree *treep,
			      struct vm_directory *dirp, UW addr,
			      UW access)
{
    struct vm_page *pagep;


    pagep = (struct vm_page *) malloc(sizeof(struct vm_page));
    if (pagep == NULL) {
	return (NULL);
    }

    pagep->addr = addr;
    return (pagep);
}


struct vm_directory *alloc_vm_directory(struct vm_tree *treep, UW addr)
{
    struct vm_directory *dirp;
    W i;


    dirp = (struct vm_directory *) malloc(sizeof(struct vm_directory));
    if (dirp == NULL) {
	return (NULL);
    }

    memset((VP)dirp, 0, sizeof(struct vm_directory));
    for (i = 0; i < MAX_PAGE_ENTRY; i++) {
	dirp->page_table[i] = NULL;
    }
    return (dirp);
}


/* destroy_vmtree - vm tree の情報を解放する
 *
 *
 */
W destroy_vmtree(struct proc * procp, struct vm_tree * treep, W unmap)
{
    W dir_index;
    W page_index;
    struct vm_directory *dirp;
    struct vm_page *pagep;
    ER error_no;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    for (dir_index = 0; dir_index < MAX_DIR_ENTRY; dir_index++) {
	dirp = treep->directory_table[dir_index];
	if (dirp) {
	    for (page_index = 0; page_index < MAX_PAGE_ENTRY; page_index++) {
		pagep = dirp->page_table[page_index];
		if (pagep) {
		    /* ページの情報をパージする */
		    if (unmap) {
			error_no = kcall->region_unmap(procp->proc_maintask,
					 (VP) (pagep->addr), PAGE_SIZE);
			if (error_no) {
			    return (error_no);
			}
		    }
		    /* ページを開放 */
		    free(pagep);
		}
	    }
	    /* ディレクトリの開放 */
	    free(dirp);
	}
    }

    /*  vmtree の root の開放 */
    free(treep);

    return (EOK);
}
