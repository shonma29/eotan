/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/i386/virtual_memory.c,v 1.26 2000/01/26 08:31:49 naniwa Exp $ */

/*
 * $Log: virtual_memory.c,v $
 * Revision 1.26  2000/01/26 08:31:49  naniwa
 * to prevent memory leak
 *
 * Revision 1.25  1999/11/10 10:30:10  naniwa
 * to support execve, etc
 *
 * Revision 1.24  1999/07/26 08:54:29  naniwa
 * fixed dup_vmap_table (), etc
 *
 * Revision 1.23  1999/07/24 04:34:04  naniwa
 * fixed release_vmap()
 *
 * Revision 1.22  1999/07/23 14:35:40  naniwa
 * implemented vunmap, vunm_reg, etc
 *
 * Revision 1.21  1999/07/09 08:20:33  naniwa
 * bug fix of vget_reg/vput_reg
 *
 * Revision 1.20  1999/04/13 04:15:12  monaka
 * MAJOR FIXcvs commit -m 'MAJOR FIX!!! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.'! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.
 *
 * Revision 1.19  1999/04/12 15:29:18  monaka
 * pointers to void are renamed to VP.
 *
 * Revision 1.18  1999/04/12 14:47:40  monaka
 * vnum_reg() is added but it returns E_NOSPT.
 *
 * Revision 1.17  1999/04/12 13:29:37  monaka
 * printf() is renamed to printk().
 *
 * Revision 1.16  1999/03/16 13:07:28  monaka
 * Modifies for source cleaning. Most of these are for avoid gcc's -Wall message.
 *
 * Revision 1.15  1999/03/15 00:25:42  monaka
 * a cast for pageent has added.
 *
 * Revision 1.14  1999/03/02 16:00:23  night
 * 田畑@KMC (yusuke@kmc.kyoto-u.ac.jp) のアドバイスによる修正。
 * (vget_reg() と同様の修正)
 * --------------------
 * この if によって決定されている delta_end はそのページ内での
 * 転送の最後のアドレスのようですが、転送の最後のページの時には
 * (p + PAGE_SIZE == align_end) が成立するはずです、このときに
 * もとの条件だとdelta_endがPAGE_SIZEになり、必要以上にコピーされて
 * しまいスタックが破壊されることが有りました。
 * --------------------
 *
 * Revision 1.13  1999/03/02 15:27:30  night
 * 田畑@KMC (yusuke@kmc.kyoto-u.ac.jp) のアドバイスによる修正。
 * --------------------
 * この if によって決定されている delta_end はそのページ内での
 * 転送の最後のアドレスのようですが、転送の最後のページの時には
 * (p + PAGE_SIZE == align_end) が成立するはずです、このときに
 * もとの条件だとdelta_endがPAGE_SIZEになり、必要以上にコピーされて
 * しまいスタックが破壊されることが有りました。
 * --------------------
 *
 * Revision 1.12  1998/02/25 12:43:51  night
 * vmap() にアクセス権を指定する引数を追加。
 * および、引数に従って、アクセス権を設定する処理を追加。
 *
 * Revision 1.11  1998/02/24 14:12:20  night
 * vget_reg()/vput_reg() でコピーする領域の先頭アドレスが
 * ページ境界に一致しているときに、コピーされないという
 * バグを修正した。
 *
 * vmap_reg() と vtor() で対象となるタスクの状態が DORMANT
 * 状態の時にエラーとみなしていたのを、正しい状態と受けいれる
 * ようにした。
 *
 * Revision 1.10  1998/02/23 14:45:51  night
 * タスクを生成するときに、すべての仮想空間を親タスク (タスクを生成したタ
 * スク) から継承するのをやめ、システム空間 (0x80000000 以降) のみを継承
 * するように変更した。
 *
 * Revision 1.9  1998/02/16 14:19:34  night
 * vcre_reg() の引数チェックを変更。
 * 以前は、スタートアドレスが 0 以下の時には、パラメータエラーに
 * していた。これだと、スタートアドレスに 0 を指定した時にエラー
 * になってしまう。スタートアドレスのチェックを外した。
 *
 * Revision 1.8  1997/09/09 13:51:35  night
 * デバッグ用の printf を削除。
 *
 * Revision 1.7  1997/08/31 14:12:40  night
 * lowlib 関係の処理を追加。
 *
 * Revision 1.6  1997/07/06 11:56:06  night
 * 定数指定で、Unsigned Long であることを示す UL を追加した。
 *
 * Revision 1.5  1997/05/14 14:09:36  night
 * vget_reg() と vput_reg() を正常に動くよう修正した。
 *
 * Revision 1.4  1997/05/12 14:36:21  night
 * デバッグ用の文を追加。
 *
 * Revision 1.3  1997/03/25 13:31:05  night
 * 関数のプロトタイプ宣言の追加および引数の不整合の修正
 *
 * Revision 1.2  1996/11/07  12:43:15  night
 * vtor() の追加および vget_reg() と vput_reg() の中身を作成した。
 *
 * Revision 1.1  1996/07/22  13:39:21  night
 * IBM PC 版 ITRON の最初の登録
 *
 * Revision 1.12  1996/01/06 16:08:03  night
 * 呼び出す関数名が間違っていたので正しく直した。
 *
 * Revision 1.11  1995/12/13 15:02:23  night
 * vmap_reg() 関数の中身を作成した。
 *
 * Revision 1.10  1995/09/21  15:51:29  night
 * ソースファイルの先頭に Copyright notice 情報を追加。
 *
 * Revision 1.9  1995/09/17  17:00:12  night
 * 余分な printf () を #ifdef notdef ... #endif で囲んだ。
 *
 * Revision 1.8  1995/09/14  04:33:09  night
 * アドレスマスク用の定数の値を変更。
 *
 * Revision 1.7  1995/05/31  22:58:00  night
 * いくつかの #ifdef DEBUG ... #endif の追加。
 * (余分なデバッグ用 printf 文を隔離した)
 *
 * Revision 1.6  1995/03/18  14:50:11  night
 * vcre_reg() 関数をコンパイルできるように修正。
 *
 * Revision 1.5  1995/02/26  14:07:40  night
 * RCS マクロ $ Header $ と $ Log $ を追加。
 *
 *
 */

/* virtual_memory.c
 *	
 *
 */

#include <string.h>
#include <core.h>
#include <mpu/config.h>
#include <mpu/memory.h>
#include <misc.h>
#include <func.h>
#include <thread.h>
#include "mpufunc.h"

static I386_PAGE_ENTRY *alloc_pagetable(W);


/* dup_vmap_table --- 指定された仮想メモリのマッピングテーブルを
 *		      コピーする。
 *		      マップテーブル自体は新しく作成する。
 *
 */
ADDR_MAP dup_vmap_table(ADDR_MAP dest)
{
    ADDR_MAP newp;
    int i;
    I386_PAGE_ENTRY *p;

    dest = (ADDR_MAP)((UW) kern_p2v(dest));
    newp = (ADDR_MAP) (palloc(1));	/* ページディレクトリのアロケート */
    memset((VP)newp, 0, PAGE_SIZE);


/* 1998/Feb/23 */
    for (i = ADDR_MAP_SIZE / 2; i < ADDR_MAP_SIZE; i++) {
	newp[i] = dest[i];	/* ページディレクトリを１エントリずつコピー */
	if (newp[i].present) {
	    /* エントリがマッピングされているならば、コピーする。 */
#if 0
	    printk("dir[%d] ", i);
#endif
	    p = (I386_PAGE_ENTRY *) (palloc(1));

#ifdef DEBUG
	    printk
		("dup_vmap_table: (VP)kern_p2v(dest[i].frame_addr << PAGE_SHIFT) = 0x%x,"
		 "(VP)p = 0x%x, PAGE_SIZE = %d\n",
		 (VP) kern_p2v(dest[i].frame_addr << PAGE_SHIFT), p,
		 PAGE_SIZE);
#endif
	    {
		int j;
		char *q, *r;

		q = (VP) kern_p2v((void*)(dest[i].frame_addr << PAGE_SHIFT));
		r = (char *) p;
		for (j = 0; j < PAGE_SIZE; j++) {
#ifdef notdef
		    printk("copy source = 0x%x, dest = 0x%x\n", q, r);
#endif
		    *r++ = *q++;
		}
	    }

#ifdef DEBUG
	    printk("dup_vmap_table: [%d]copy 0x%x -> 0x%x\n",
		   i, (VP) kern_p2v(dest[i].frame_addr << PAGE_SHIFT), (VP) p);
#endif
	    newp[i].frame_addr = (UW)kern_v2p(p) >> PAGE_SHIFT;
	}
    }
#if 0
    printk("\n");
#endif
    return (newp);
}

/***********************************************************************
 * release_vmap --- 指定したアドレスマップテーブルをすべて解放する。
 *
 */
extern ER release_vmap(ADDR_MAP dest)
{
    I386_PAGE_ENTRY *p;
    W i, j;
    UW ppage;

    dest = (ADDR_MAP) kern_p2v(dest);
    for (i = 0; i < ADDR_MAP_SIZE; i++) {
	if (dest[i].present) {
	    p = (I386_PAGE_ENTRY *) (dest[i].frame_addr << PAGE_SHIFT);
	    if ((UW) p <= KERNEL_SIZE) {
		p = (I386_PAGE_ENTRY*)(kern_p2v(p));
	    }
	    if (i < ADDR_MAP_SIZE / 2) {
		for (j = 0; j < PAGE_SIZE / sizeof(I386_PAGE_ENTRY); j++) {
		    if (p[j].present) {
			ppage = (UW)kern_v2p((void*)(p[j].frame_addr << PAGE_SHIFT));
			pfree((VP) ppage, 1);
		    }
		}
	    }
	    p = (I386_PAGE_ENTRY*)(kern_v2p(p));
	    pfree((VP) p, 1);
	}
    }
    dest = (ADDR_MAP) kern_v2p(dest);
    pfree((VP) dest, 1);

    return E_OK;
}


/*************************************************************************
 * vmap --- 仮想メモリのマッピング
 *
 * 引数：	task	マッピングの対象となるタスク
 *		vpage	仮想メモリアドレス
 *		ppage	物理メモリアドレス
 *		accmode	アクセス権 (0 = kernel, 1 = user)
 *
 * 返値：	TRUE	成功
 *		FALSE	失敗
 *
 * 処理：	引数で指定された仮想メモリを物理メモリに割り当てる
 *
 */
BOOL vmap(T_TCB * task, UW vpage, UW ppage, W accmode)
{
    I386_DIRECTORY_ENTRY *dirent, *dp;
    I386_PAGE_ENTRY *pageent, *pp;
    UW dirindex;
    UW pageindex;

    if (vpage >= MIN_KERNEL)
	return TRUE;

#ifdef DEBUG
    printk("[%d] vmap: 0x%x -> 0x%x\n", task->tskid, ppage, vpage);
#endif				/* DEBUG */
/*  task->context.cr3 &= 0x7fffffff; */
    dirent = (I386_DIRECTORY_ENTRY *) (task->mpu.context.cr3);
    dirent = (I386_DIRECTORY_ENTRY*)(kern_p2v(dirent));
    dirindex = vpage & DIR_MASK;
    dirindex = dirindex >> DIR_SHIFT;
    pageindex = (vpage & PAGE_MASK) >> PAGE_SHIFT;

#ifdef DEBUG
    printk("dirindex = %d, pageindex = %d\n", dirindex, pageindex);
#endif				/* DEBUG */
    if (dirent[dirindex].present != 1) {
	/* ページディレクトリのエントリは空だった。
	 * 新しくページディレクトリのエントリを埋める。
	 */
	pageent = (I386_PAGE_ENTRY *) alloc_pagetable(accmode);
	if (pageent == NULL) {
	    return (FALSE);
	}
#ifdef DEBUG
	printk("dir alloc(newp). frame = 0x%x ",
	       ((UW) pageent & 0x0fffffff) >> PAGE_SHIFT);
#endif				/* DEBUG */
	/*      dirent[dirindex].frame_addr = ((UW)pageent & 0x0fffffff) >> PAGE_SHIFT; */
	dp = &dirent[dirindex];
	dp->frame_addr = (UW)kern_v2p(pageent) >> PAGE_SHIFT;
	dp->present = 1;
	dp->read_write = 1;
	dp->u_and_s = ((accmode & ACC_USER) ? 1 : 0);
	dp->zero2 = 0;
	dp->access = 0;
	dp->dirty = 0;
	dp->user = accmode;
	dp->zero1 = 0;
    } else {
	pageent =
	    (I386_PAGE_ENTRY *) (dirent[dirindex].frame_addr << PAGE_SHIFT);
#ifdef DEBUG
	printk("dir alloc(old). frame = 0x%x ",
	       dirent[dirindex].frame_addr);
#endif				/* DEBUG */
    }

    if ((UW) pageent <= KERNEL_SIZE) {
	pageent = (I386_PAGE_ENTRY*)(kern_p2v(pageent));
    }

    if (pageent[pageindex].present == 1) {
	/* 既にページが map されていた */
	printk("vmap: vpage %x has already mapped\n", vpage);
	/*    return(FALSE); */
    }
    pp = &pageent[pageindex];
    pp->frame_addr = (UW)kern_v2p((void*)ppage) >> PAGE_SHIFT;
    pp->present = 1;
    pp->read_write = 1;
    pp->u_and_s = ((accmode & ACC_USER) ? 1 : 0);
    pp->zero2 = 0;
    pp->access = 0;
    pp->dirty = 0;
    pp->zero1 = 0;
    pp->user = 0;

#ifdef DEBUG
    printk("pageindex = %d, frame = 0x%x\n", pageindex,
	   pageent[pageindex].frame_addr);
#endif				/* DEBUG */
    return (TRUE);
}

/* 仮想メモリのアンマップ
 *
 * 引数:	virtual	仮想メモリアドレス
 *
 */
ER vunmap(T_TCB * task, UW vpage)
{
    I386_DIRECTORY_ENTRY *dirent;
    I386_PAGE_ENTRY *pageent;
    UW dirindex;
    UW pageindex;
    UW ppage;
/*    ER errno;*/

    dirent = (I386_DIRECTORY_ENTRY *) (task->mpu.context.cr3);
    dirent = (I386_DIRECTORY_ENTRY*)(kern_p2v(dirent));
    dirindex = vpage & DIR_MASK;
    dirindex = dirindex >> DIR_SHIFT;
    pageindex = (vpage & PAGE_MASK) >> PAGE_SHIFT;

/*
  dirindex = vpage / (PAGE_SIZE * PAGE_SIZE);
*/
/*
  pageindex = (vpage % (PAGE_SIZE * PAGE_SIZE * PAGE_SIZE));
*/

#ifdef DEBUG
    printk("dirindex = %d, pageindex = %d\n", dirindex, pageindex);
#endif				/* DEBUG */
    if (dirent[dirindex].present != 1) {
	/* ページディレクトリのエントリは空だった。
	 */
	return (FALSE);
    } else {
	pageent =
	    (I386_PAGE_ENTRY *) (dirent[dirindex].frame_addr << PAGE_SHIFT);
    }

    if ((UW) pageent <= KERNEL_SIZE) {
	pageent = (I386_PAGE_ENTRY*)(kern_p2v(pageent));
    }

    ppage = (UW)kern_v2p((void*)(pageent[pageindex].frame_addr << PAGE_SHIFT));
    /*TODO handle error */
    /*
    errno = pfree((VP) ppage, 1);
    if (errno)
	return (FALSE);
    */
    pfree((VP) ppage, 1);
    pageent[pageindex].present = 0;
    return (TRUE);
}



/*************************************************************************
 * alloc_pagetable --- ページテーブルエントリの作成
 *
 * 引数：
 *
 * 返値：
 *
 * 処理：
 *
 */
static I386_PAGE_ENTRY *alloc_pagetable(W accmode)
{
    I386_PAGE_ENTRY *newp, *pp;
    W i;

    newp = (I386_PAGE_ENTRY *) palloc(1);
    if (newp == NULL) {
	return (NULL);
    }
    memset(newp, 0, PAGE_SIZE);
    for (i = 0, pp = newp; i < PAGE_SIZE / sizeof(I386_PAGE_ENTRY);
	 ++i, ++pp) {
	pp->present = 0;
	pp->read_write = 1;
	pp->u_and_s = ((accmode & ACC_USER) ? 1 : 0);
	pp->zero2 = 0;
	pp->access = 0;
	pp->dirty = 0;
	pp->user = 0;
	pp->zero1 = 0;
	pp->frame_addr = 0;
    }
    return (newp);
}


/* vtor - 仮想メモリアドレスをカーネルから直接アクセス可能なアドレスに変換する
 *
 */
UW vtor(ID tskid, UW addr)
{
    T_TCB *taskp;
    UW result;

    taskp = (T_TCB *) get_thread_ptr(tskid);
    if (taskp->tskstat == TTS_NON)
    {
	return (UW)(NULL);
    }

    result = (UW)getPageAddress((PTE*)kern_p2v((void*)(taskp->mpu.context.cr3)),
	    (void*)addr);

    return result? (result | getOffset((void*)addr)):result;
}



/*
 * リージョンの作成
 *
 * 各タスクは、リージョンの配列をもっている。
 * その中で使っていないエントリを選び、引数で指定した情報を入れる。
 *
 * この関数の中では、物理メモリをマッピングするような処理はしない。
 * 単に新しいリージョンをひとつ割り当てるだけである。
 * もし、リージョンを生成したときに物理メモリを割り当てたいときには、
 * region_create を実行したあとに region_map を実行する必要がある。
 *
 */
ER region_create(ID id,		/* task ID */
	    ID rid,		/* region number */
	    VP start,		/* リージョンの開始アドレス */
	    W min,		/* リージョンの最小(初期)サイズ */
	    W max,		/* リージョンの最大サイズ */
	    UW perm)		/* リージョンのパーミッション */
{				/* リージョン内でページフォールトが発生したと */
    /* きの処理の指定 */
    T_TCB *taskp;
    T_REGION *regp;
#ifdef DEBUG
    printk("region_create %d %d %x %x %x %x %x\n", id, rid, start, min, max,
	   perm);
#endif

    /*
     * 引数のチェックを行う。
     * もし引数の値がおかしい場合には、E_PAR のエラー番号を返す。
     */
/*
  if (start <= 0)	return (E_PAR);
*/
    if (min < 0)
	return (E_PAR);
    if (max <= 0)
	return (E_PAR);
    if (min > max)
	return (E_PAR);

    /*
     * タスク ID から該当するタスクのコンテキストへの
     * ポインタを取り出す。
     */
    taskp = get_thread_ptr(id);
    if (taskp == NULL) {
	/*
	 * 引数で指定した ID をもつタスクは存在していない。
	 * E_OBJ を返す。
	 */
	return (E_OBJ);
    }

    if (rid < 0 || rid >= MAX_REGION)
	return (E_PAR);
    if (taskp->regions[rid].permission != 0) {
	return (E_ID);
    }
    regp = &(taskp->regions[rid]);

    /*
     * リージョン情報の設定。
     * リージョンエントリへは、引数の値をそのまま入れずに以下のような処
     * 理を行う。
     *    start           ページサイズで切り捨てる
     *    min_size        ページサイズで切り上げる
     *    max_size        ページサイズで切り上げる
     *    permission      そのまま
     */
    regp->start_addr = (VP) CUTDOWN(start, PAGE_SIZE);
    regp->min_size = ROUNDUP(min, PAGE_SIZE);
    regp->max_size = ROUNDUP(max, PAGE_SIZE);
    regp->permission = perm;

    /*
     * 処理は正常に終了した。
     */
    return (E_OK);
}

/*
 * リージョンの破棄
 *
 * 引数 start で指定したアドレス領域を管理するリージョンを削除する。 
 * 削除したリージョンに含まれる領域中のデータは破棄する。
 *
 * start の値で指定したアドレスは、リージョンの先頭アドレスである必要
 * はない。リージョン内のアドレスならば、どのリージョンを指定したかを
 * システムコール内で判断する。
 *
 */
ER region_destroy(ID id, ID rid)
{
    T_TCB *taskp;

#ifdef DEBUG
    printk("region_destroy %d %d\n", id, rid);
#endif
    taskp = get_thread_ptr(id);
    if (taskp == NULL) {
	/*
	 * 引数で指定した ID をもつタスクは存在していない。
	 * E_OBJ を返す。
	 */
	return (E_OBJ);
    }
    if (rid < 0 || rid >= MAX_REGION)
	return (E_PAR);
    taskp->regions[rid].permission = 0;
    return (E_OK);
}

/*
 * リージョン内の仮想ページへ物理メモリを割り付ける。
 *
 * 引数で指定したアドレス領域に物理メモリを割り付ける。
 *
 * 複数のページに相当するサイズが指定された場合、全てのページがマップ
 * 可能のときのみ物理メモリを割り付ける。その他の場合は割り付けない。
 *
 * マップする物理メモリのアドレスは指定できない。中心核が仮想メモリに
 * 割り付ける物理メモリを適当に割り振る。
 *
 *
 * 返り値
 *
 * 以下のエラー番号が返る。
 *	E_OK     リージョンのマップに成功  
 *	E_NOMEM  (物理)メモリが不足している
 *	E_NOSPT  本システムコールは、未サポート機能である。
 *	E_PAR	 引数がおかしい
 *
 */
ER region_map(ID id, VP start, UW size, W accmode)
    /* 
     * id        タスク ID
     * start     マップする仮想メモリ領域の先頭アドレス
     * size      マップする仮想メモリ領域の大きさ(バイト単位)
     * accmode   マップする仮想メモリ領域のアクセス権を指定
     *           (ACC_KERNEL = 0, ACC_USER = 1)
     */
{
    T_TCB *taskp;
    UW counter, i;
    VP pmem;
    ER res;
    T_REGION *regp;
    UW newsize;

    taskp = (T_TCB *) get_thread_ptr(id);
    if (taskp->tskstat == TTS_NON)
    {
#ifdef DEBUG
	printk("region_map: taskp->tskstat = %d\n", taskp->tskstat);	/* */
#endif
	return (E_PAR);
    }

    regp = NULL;
    for (counter = 0; counter < MAX_REGION; counter++) {
	if ((taskp->regions[counter].permission != 0) &&
	    ((UW) taskp->regions[counter].start_addr <= (UW) start) &&
	    ((UW) start < ((UW) taskp->regions[counter].start_addr +
			   taskp->regions[counter].max_size))) {
	    regp = &(taskp->regions[counter]);
	    break;
	}
    }

    size = pages(size);
    start = (VP)(CUTDOWN(start, PAGE_SIZE));
    if (pmemfree() < size)
	return (E_NOMEM);
    res = E_OK;
    for (counter = 0; counter < size; counter++) {
	pmem = palloc(1);
	if (pmem == NULL) {
	    res = E_NOMEM;
	    break;
	}
	if (vmap(taskp, ((UW) start + (counter << PAGE_SHIFT)),
		 (UW) kern_v2p(pmem), accmode) == FALSE) {
	    pfree((VP) kern_v2p(pmem), 1);
	    res = E_SYS;
	    break;
	}
    }
    if (res != E_OK) {
	for (i = 0; i < counter; ++i) {
	    vunmap(taskp, (UW) start + (i << PAGE_SHIFT));
	}
    } else {
	if (regp) {
	    if (((UW) start) + (size << PAGE_SHIFT) > (UW) regp->start_addr) {
		newsize =
		    (UW) start + (size << PAGE_SHIFT) - (UW) regp->start_addr;
		if (newsize > regp->min_size) {
#ifdef DEBUG
		    printk
			("region_map: new region size %x:%x->%x  (%x %x)\n",
			 regp->start_addr, regp->min_size, newsize, start,
			 size);
#endif
		    regp->min_size = newsize;
		    if (regp->min_size > regp->max_size) {
			printk
			    ("[WARNING] region_map: min_size exceeds max_size\n");
		    }
		}
	    }
	}
    }
    return (res);
}

/*
 *
 */
ER region_unmap(ID id, VP start, UW size)
{
    T_TCB *taskp;
    UW counter;
    T_REGION *regp;
    UW newsize;

    taskp = (T_TCB *) get_thread_ptr(id);
    if (taskp->tskstat == TTS_NON) {
	return (E_PAR);
    }

    regp = NULL;
    for (counter = 0; counter < MAX_REGION; counter++) {
	if ((taskp->regions[counter].permission != 0) &&
	    ((UW) taskp->regions[counter].start_addr <= (UW) start) &&
	    ((UW) start < ((UW) taskp->regions[counter].start_addr +
			   taskp->regions[counter].max_size))) {
	    regp = &(taskp->regions[counter]);
	    break;
	}
    }

    size = ROUNDUP(size, PAGE_SIZE);
    start = (VP)(CUTDOWN(start, PAGE_SIZE));
    for (counter = 0; counter < (size >> PAGE_SHIFT); counter++) {
	if (vunmap(taskp, ((UW) start + (counter << PAGE_SHIFT))) == FALSE) {
	    return (E_SYS);
	}
    }

    if (regp) {
	if (start >= regp->start_addr) {
	    newsize = (UW) (start - regp->start_addr);
	    if (newsize + size == regp->min_size) {
		regp->min_size = newsize;
#ifdef DEBUG
		printk("region_unmap: new region size %x\n", newsize);
#endif
	    }
	}
    }
    return (E_OK);
}

/*
 * 指定したタスクのもつリージョンを複製する。
 *
 * 複製したリージョンは、全く別のものとして扱われる。
 * src, dst のどちらかのタスクがリージョンの領域を変更しても、もう片方
 * のタスクは影響を受けない。
 *
 */
ER region_duplicate(ID src, ID dst)
    /* src    複製するリージョンをもつタスク
     * dst    リージョンの複製先のタスク
     */
{
    T_TCB *taskp, *dstp;

#ifdef DEBUG
    printk("region_duplicate %d %d\n", src, dst);
#endif
    taskp = (T_TCB *) get_thread_ptr(src);
    if (taskp->tskstat == TTS_NON) {
	return (E_PAR);
    }
    dstp = (T_TCB *) get_thread_ptr(dst);
    if (dstp->tskstat == TTS_NON) {
	return (E_PAR);
    }

    dstp->regions[TEXT_REGION] = taskp->regions[TEXT_REGION];
    dstp->regions[DATA_REGION] = taskp->regions[DATA_REGION];
    dstp->regions[HEAP_REGION] = taskp->regions[HEAP_REGION];
    return (E_OK);
}

/*
 * リージョンからの読み込み
 *
 * 任意のタスクの仮想メモリ領域からデータを読み込む。
 * ページアウトなどに使用する。
 *
 *
 * 返り値
 *
 * 以下のエラー番号が返る。
 *
 *	E_OK     成功  
 *	E_ID     リージョンをもつタスク
 *	E_NOSPT  本システムコールは、未サポート機能である
 *
 */
ER region_get(ID id, VP start, UW size, VP buf)
    /*
     * id     リージョンを持つタスク
     * start  読み込む領域の先頭アドレス
     * size   リージョンから読み込むサイズ
     * buf    リージョンから読み込んだデータを収めるバッファ
     */
{
    UW offset;
    UW align_start;
    UW align_end;
    UW paddr;
    UW copysize;
    UW bufoffset;
    UW p;
    UW delta_start, delta_end;


    if (id < 0)
	return (E_PAR);
    if (size < 0)
	return (E_PAR);
    else if (size == 0)
	return (E_OK);
    if (buf == NULL)
	return (E_PAR);

    align_start = CUTDOWN(start, PAGE_SIZE);
    align_end = ROUNDUP(start + size, PAGE_SIZE);

    bufoffset = 0;

    for (p = align_start; p < align_end; p += PAGE_SIZE) {
	paddr = (UW) vtor(id, p);	/* 物理メモリアドレスの取得 */
	if (paddr == (UW)NULL) {
	    return (E_PAR);
	}

	if (p == align_start) {
	    offset = (UW) paddr + ((UW) start - align_start);
	    delta_start = (UW) start - align_start;
	} else {
	    offset = (UW) paddr;
	    delta_start = 0;
	}

	if ((p + PAGE_SIZE) >= align_end) {
	    delta_end = align_end - p;
	} else {
	    delta_end = PAGE_SIZE;
	}

	copysize = delta_end - delta_start;
	if (copysize > size)
	    copysize = size;
	memcpy(&((B*)buf)[bufoffset], (VP)offset, copysize);
	bufoffset += copysize;
	size -= copysize;
    }

    return (E_OK);
}

/*
 * リージョンへの書き込み
 *
 * 任意のタスクの仮想メモリ領域にデータを書き込む。
 * ページインなどに使用できる。
 *
 *
 * 返り値
 *
 * 以下のエラー番号が返る。
 *
 *	E_OK     リージョンへの書き込みに成功  
 *	E_ID     引数 id に対応したタスクは存在しない
 *	E_NOSPT  本システムコールは、未サポート機能である
 *
 */
ER region_put(ID id, VP start, UW size, VP buf)
    /*
     * id     リージョンを持つタスク
     * start  書き込む領域の先頭アドレス
     * size   リージョンに書き込むサイズ
     * buf    リージョンに書き込むデータ
     */
{
    T_TCB *th;

    if (size == 0)
	return (E_OK);
    if (buf == NULL)
	return (E_PAR);

    th = get_thread_ptr(id);
    if (!th)
	return E_PAR;

    if (th->tskstat == TTS_NON)
	return E_PAR;

    return vmemcpy(th, start, buf, size);
}

/*
 * リージョンの情報を取得する。
 *
 * リージョン情報としては次のものが考えられる。
 *
 *	リージョンの先頭仮想アドレス
 *	リージョンのサイズ
 *	プロテクト情報
 * 
 * 返り値
 *
 * 以下のエラー番号が返る。
 *
 *	E_OK     リージョンの情報の取得に成功  
 *	E_ID     引数 id で指定したタスクは存在しない
 *	E_NOSPT  本システムコールは、未サポート機能である
 *
 */
ER region_get_status(ID id, ID rid, VP stat)
    /*
     * id     リージョンをもつタスク
     * rid    region number
     * stat   リージョン情報が入る(リージョン情報の詳細は未決定である) 
     */
{
    T_REGION *regp = stat;
    T_TCB *taskp;

    taskp = get_thread_ptr(id);
    if (taskp == NULL) {
	/*
	 * 引数で指定した ID をもつタスクは存在していない。
	 * E_OBJ を返す。
	 */
	return (E_OBJ);
    }

    if (rid < 0 || rid >= MAX_REGION)
	return (E_PAR);
    if (taskp->regions[rid].permission == 0)
	return (E_ID);

    regp->start_addr = taskp->regions[rid].start_addr;
    regp->min_size = taskp->regions[rid].min_size;
    regp->max_size = taskp->regions[rid].max_size;
    regp->permission = taskp->regions[rid].permission;
    return (E_OK);
}
