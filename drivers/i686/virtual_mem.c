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
#include <mm/segment.h>
#include <mpu/config.h>
#include <mpu/memory.h>
#include <func.h>
#include <thread.h>
#include "mpufunc.h"

/***********************************************************************
 *	directory table entry.
 */
typedef struct
{
  UW	present:1;
  UW	read_write:1;
  UW	u_and_s:1;
  UW	zero2:2;
  UW	access:1;		/* アクセスチェック用ビット */
  UW	dirty:1;		/* 書き込みチェック用ビット */
  UW	zero1:2;
  UW	user:3;
  UW	frame_addr:20;
} I386_DIRECTORY_ENTRY;

/***********************************************************************
 *	page table entry.
 */
typedef struct
{
  UW	present:1;
  UW	read_write:1;
  UW	u_and_s:1;
  UW	zero2:2;
  UW	access:1;
  UW	dirty:1;
  UW	zero1:2;
  UW	user:3;
  UW	frame_addr:20;
} I386_PAGE_ENTRY;

static BOOL vmap(thread_t * task, UW vpage, UW ppage, W accmode);
static ER vunmap(thread_t * task, UW vpage);



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
static BOOL vmap(thread_t * task, UW vpage, UW ppage, W accmode)
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
    dirent = (I386_DIRECTORY_ENTRY *) (task->mpu.cr3);
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
	pageent = (I386_PAGE_ENTRY *) palloc();
	if (pageent == NULL) {
	    return (FALSE);
	}
#ifdef DEBUG
	printk("dir alloc(newp). frame = 0x%x ",
	       ((UW) pageent & 0x0fffffff) >> PAGE_SHIFT);
#endif				/* DEBUG */
	dp = &dirent[dirindex];
	dp->frame_addr = (UW)kern_v2p(pageent) >> PAGE_SHIFT;
	dp->present = 1;
	dp->read_write = 1;
	dp->u_and_s = accmode;
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
    pp->u_and_s = accmode;
    pp->zero2 = 0;
    pp->access = 0;
    pp->dirty = 0;
    pp->zero1 = 0;
    pp->user = 0;

#ifdef DEBUG
    printk("pageindex = %d, frame = 0x%x\n", pageindex,
	   pageent[pageindex].frame_addr);
#endif				/* DEBUG */
    context_reset_page_cache(task, (VP)vpage);

    return (TRUE);
}

/* 仮想メモリのアンマップ
 *
 * 引数:	virtual	仮想メモリアドレス
 *
 */
static ER vunmap(thread_t * task, UW vpage)
{
    I386_DIRECTORY_ENTRY *dirent;
    I386_PAGE_ENTRY *pageent;
    UW dirindex;
    UW pageindex;
    UW ppage;

    dirent = (I386_DIRECTORY_ENTRY *) (task->mpu.cr3);
    dirent = (I386_DIRECTORY_ENTRY*)(kern_p2v(dirent));
    dirindex = vpage & DIR_MASK;
    dirindex = dirindex >> DIR_SHIFT;
    pageindex = (vpage & PAGE_MASK) >> PAGE_SHIFT;

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
    pfree((VP) ppage);
    pageent[pageindex].present = 0;
    context_reset_page_cache(task, (VP)vpage);
    return (TRUE);
}



/* vtor - 仮想メモリアドレスをカーネルから直接アクセス可能なアドレスに変換する
 *
 */
UW vtor(ID tskid, UW addr)
{
    thread_t *taskp;
    UW result;

    taskp = (thread_t *) get_thread_ptr(tskid);
    if (!taskp)
    {
	return (UW)(NULL);
    }

    result = (UW)getPageAddress((PTE*)kern_p2v((void*)(taskp->mpu.cr3)),
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
    thread_t *taskp;
    mm_segment_t *regp;
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
    if (max < 0)
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

    if (rid < 0 || rid > seg_stack)
	return (E_PAR);
    if (taskp->segments[rid].attr != attr_nil) {
	return (E_OBJ);
    }
    regp = &(taskp->segments[rid]);

    /*
     * リージョン情報の設定。
     * リージョンエントリへは、引数の値をそのまま入れずに以下のような処
     * 理を行う。
     *    start           ページサイズで切り捨てる
     *    min_size        ページサイズで切り上げる
     *    max_size        ページサイズで切り上げる
     *    permission      そのまま
     */
    regp->addr = (VP) pageRoundDown((UW)start);
    regp->len = pageRoundUp(min);
    regp->max = pageRoundUp(max);
    regp->attr = perm;

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
ER region_destroy(ID id)
{
    thread_t *taskp;
    ID rid;

#ifdef DEBUG
    printk("region_destroy %d\n", id);
#endif
    taskp = get_thread_ptr(id);
    if (taskp == NULL) {
	/*
	 * 引数で指定した ID をもつタスクは存在していない。
	 * E_OBJ を返す。
	 */
	return (E_OBJ);
    }
    for (rid = seg_code; rid <= seg_heap; rid++) {
    	region_unmap(id, taskp->segments[rid].addr,
		taskp->segments[rid].len);
	taskp->segments[rid].attr = attr_nil;
    }

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
    thread_t *taskp;
    UW counter, i;
    VP pmem;
    ER res;
    mm_segment_t *regp;
    UW newsize;

    taskp = (thread_t *) get_thread_ptr(id);
    if (!taskp)
    {
#ifdef DEBUG
	printk("region_map: taskp->tskstat = %d\n", taskp->tskstat);	/* */
#endif
	return (E_PAR);
    }

    regp = find_region(taskp, start);
    size = pages(size);
    start = (VP)pageRoundDown((UW)start);
    if (pmemfree() < size)
	return (E_NOMEM);
    res = E_OK;
    for (counter = 0; counter < size; counter++) {
	pmem = palloc();
	if (pmem == NULL) {
	    res = E_NOMEM;
	    break;
	}
	if (vmap(taskp, ((UW) start + (counter << PAGE_SHIFT)),
		 (UW) kern_v2p(pmem), accmode) == FALSE) {
	    pfree((VP) kern_v2p(pmem));
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
	    if (((UW) start) + (size << PAGE_SHIFT) > (UW) regp->addr) {
		newsize =
		    (UW) start + (size << PAGE_SHIFT) - (UW) regp->addr;
		if (newsize > regp->len) {
#ifdef DEBUG
		    printk
			("region_map: new region size %x:%x->%x  (%x %x)\n",
			 regp->start_addr, regp->min_size, newsize, start,
			 size);
#endif
		    regp->len = newsize;
		    if (regp->len > regp->max) {
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
    thread_t *taskp;
    UW counter;
    mm_segment_t *regp;
    UW newsize;

    taskp = (thread_t *) get_thread_ptr(id);
    if (!taskp) {
	return (E_PAR);
    }

    regp = find_region(taskp, start);
    size = pageRoundUp(size);
    start = (VP)pageRoundDown((UW)start);
    for (counter = 0; counter < (size >> PAGE_SHIFT); counter++) {
	if (vunmap(taskp, ((UW) start + (counter << PAGE_SHIFT))) == FALSE) {
	    return (E_SYS);
	}
    }

    if (regp) {
	if (start >= regp->addr) {
	    newsize = (UW) (start - regp->addr);
	    if (newsize + size == regp->len) {
		regp->len = newsize;
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
    thread_t *taskp, *dstp;

#ifdef DEBUG
    printk("region_duplicate %d %d\n", src, dst);
#endif
    taskp = (thread_t *) get_thread_ptr(src);
    if (!taskp) {
	return (E_PAR);
    }
    dstp = (thread_t *) get_thread_ptr(dst);
    if (!dstp) {
	return (E_PAR);
    }

    dstp->segments[seg_code] = taskp->segments[seg_code];
    dstp->segments[seg_data] = taskp->segments[seg_data];
    dstp->segments[seg_heap] = taskp->segments[seg_heap];
    return copy_user_pages(dstp->mpu.cr3, taskp->mpu.cr3,
	    (pageRoundUp(dstp->segments[seg_code].len) >> BITS_OFFSET)
	    + (pageRoundUp(dstp->segments[seg_data].len) >> BITS_OFFSET)
	    + (pageRoundUp(dstp->segments[seg_heap].len) >> BITS_OFFSET)
    );
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
 *	E_NOEXS  リージョンをもつタスクは存在しない
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

    align_start = pageRoundDown((UW)start);
    align_end = pageRoundUp((UW)start + size);

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
 *	E_NOEXS  引数 id に対応したタスクは存在しない
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
    thread_t *th;

    if (size == 0)
	return (E_OK);
    if (buf == NULL)
	return (E_PAR);

    th = get_thread_ptr(id);
    if (!th)
	return E_NOEXS;

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
 *	E_NOEXS  引数 id で指定したタスクは存在しない
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
    mm_segment_t *regp = stat;
    thread_t *taskp;

    taskp = get_thread_ptr(id);
    if (taskp == NULL) {
	/*
	 * 引数で指定した ID をもつタスクは存在していない。
	 * E_NOEXS を返す。
	 */
	return (E_NOEXS);
    }

    if (rid < 0 || rid > seg_stack)
	return (E_PAR);
    if (taskp->segments[rid].attr == attr_nil)
	return (E_OBJ);

    *regp = taskp->segments[rid];
    return (E_OK);
}
