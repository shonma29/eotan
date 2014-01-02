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
#include <mpu/memory.h>
#include <func.h>
#include <thread.h>
#include "mpufunc.h"


/* vtor - 仮想メモリアドレスをカーネルから直接アクセス可能なアドレスに変換する
 *
 */
UW vtor(thread_t *taskp, UW addr)
{
    UW result = (UW)getPageAddress((PTE*)kern_p2v((void*)(taskp->mpu.cr3)),
	    (void*)addr);

    return result? (result | getOffset((void*)addr)):result;
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
ER region_map(VP page_table, VP start, UW size, W accmode)
    /* 
     * page_table        仮想メモリマップ
     * start     マップする仮想メモリ領域の先頭アドレス
     * size      マップする仮想メモリ領域の大きさ(バイト単位)
     * accmode   マップする仮想メモリ領域のアクセス権を指定
     *           (ACC_KERNEL = 0, ACC_USER = 1)
     */
{
    ER res;

printk("region_map: %x %p %x %x\n", page_table, start, size, accmode);
    size = pages(size);
    start = (VP)pageRoundDown((UW)start);
    if (pmemfree() < size)
	return (E_NOMEM);
    res = map_user_pages(page_table, start, size);
    if (res != E_OK) {
	unmap_user_pages(page_table, start, size);
    }
    return (res);
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
    thread_t *taskp;

    if (id < 0)
	return (E_PAR);
    if (size < 0)
	return (E_PAR);
    else if (size == 0)
	return (E_OK);
    if (buf == NULL)
	return (E_PAR);

    taskp = (thread_t *) get_thread_ptr(id);
    if (!taskp)
	return (E_NOEXS);

    return vmemcpy2(taskp, buf, start, size);
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
