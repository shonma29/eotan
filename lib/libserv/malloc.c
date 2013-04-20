/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/* @(#) $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/kernlib/malloc.c,v 1.6 2000/02/13 09:07:21 naniwa Exp $ */

/* 
 * $Log: malloc.c,v $
 * Revision 1.6  2000/02/13 09:07:21  naniwa
 * minor fix
 *
 * Revision 1.5  2000/02/06 11:06:22  naniwa
 * try to fix log comment
 *
 * Revision 1.4  2000/02/06 09:12:34  naniwa
 * minor fix
 *
 * Revision 1.3  2000/01/26 08:33:11  naniwa
 * minor fix
 *
 * Revision 1.2  1999/07/26 08:55:37  naniwa
 * fixed malloc()
 *
 * Revision 1.1  1999/04/18 17:48:33  monaka
 * Port-manager and libkernel.a is moved to ITRON. I guess it is reasonable. At least they should not be in BTRON/.
 *
 * Revision 1.5  1998/02/25 12:33:20  night
 * vmap_reg () の引数の数がひとつ増えたことによる変更。
 *
 * Revision 1.4  1997/10/24 14:03:53  night
 * init_malloc に引数追加。
 *
 * Revision 1.3  1997/09/02 14:40:54  night
 * printf を呼び出さないようにした。
 * (プログラムによっては、printf をサポートしないため)
 *
 * Revision 1.2  1997/08/31 14:09:42  night
 * malloc 関係の処理を正常に動くようにした。
 *
 * Revision 1.1  1997/07/06 13:09:43  night
 * 最初の登録
 *
 * Revision 1.5  1997/07/02 13:10:32  night
 * malloc の修正。
 *
 * Revision 1.4  1997/06/29 15:44:32  night
 * コメントの変更
 *
 * ・漢字コードを SJIS から EUC に変更した。
 * ・RCS の Log マクロ入りのコメントを追加した(まだ入れていなかったファイルのみ)。
 *
 * Revision 1.3  1997/06/29 15:33:56  night
 * コメントの修正 (rcs の Log マクロの追加)
 *
 *
 */

#include <string.h>
#include <core.h>
#include <mpu/config.h>
#include <mpu/memory.h>
#include "../../kernel/config.h"
#include "../../kernel/region.h"
#include "libserv.h"

#define MEMORY_CLICK		(PAGE_SIZE * 16)


struct alloc_entry_t {
    struct alloc_entry_t *next;
    struct alloc_entry_t *before;
    UW size;			/* alloc_data のサイズが入る */
    B alloc_data[0];
};


static UW last_page, start_page;
static struct alloc_entry_t alloc_list;
static struct alloc_entry_t *free_list;
static struct alloc_entry_t *pivot;

static VP get_system_memory(UW size);


/* malloc 機構の初期化
 *
 */
ER init_malloc(UW free_memory_erea, UW max)
{
    ID mytid;
    struct alloc_entry_t *p;
    ER err;
    UW pages = (max + PAGE_SIZE - 1) / PAGE_SIZE;

    last_page = free_memory_erea;
    start_page = last_page = pageRoundUp(last_page);

    err = get_tid(&mytid);
    if (err != E_OK) {
	return (err);
    }
    err = vcre_reg(mytid, HEAP_REGION, (VP) last_page, pages, pages,
		   VPROT_READ | VPROT_READ | VPROT_USER);
    if (err) {
	return (err);
    }

    alloc_list.size = 0;
    p = (struct alloc_entry_t *) get_system_memory(MEMORY_CLICK);
    if (p == NULL) {
	return (E_NOMEM);
    }

    p->size = MEMORY_CLICK - sizeof(struct alloc_entry_t);

#ifdef notdef
    dbg_printf
	("MEMORY_CLICK(%d) - sizeof (struct alloc_entry_t) (%d) = %d\n",
	 MEMORY_CLICK, sizeof(struct alloc_entry_t),
	 MEMORY_CLICK - sizeof(struct alloc_entry_t));
#endif

    p->next = &alloc_list;
    p->before = &alloc_list;
    alloc_list.next = p;
    alloc_list.before = p;
    free_list = &alloc_list;
    pivot = &alloc_list;

#ifdef notdef
    dbg_printf
	("init_malloc: alloc_list = 0x%x, alloc_list.next = 0x%x, alloc_list.before = 0x%x\n",
	 &alloc_list, alloc_list.next, alloc_list.before);
    dbg_printf
	("init_malloc: alloc_list.next->next = 0x%x, alloc_list.next->before = 0x%x\n",
	 alloc_list.next->next, alloc_list.next->before);
    dbg_printf("init_malloc: p = 0x%x, p->next = 0x%x, p->before = 0x%x\n",
	       p, p->next, p->before);
#endif
    return (E_OK);
}


VP malloc(UW size)
{
    struct alloc_entry_t *p;
    struct alloc_entry_t *newmem;
    int alloc_size;
    extern void free(VP addr);

    if (size <= 0) {
	return (NULL);
    }

  retry:
    p = pivot;
    do {
#ifdef notdef
	dbg_printf("malloc: search freememory: 0x%x, size = %d\n", p,
		   p->size);
#endif
	if (p->size == size) {
	    pivot = p->next;
	    p->before->next = p->next;
	    p->next->before = p->before;
#ifdef notdef
	    dbg_printf("malloc: return 0x%x\n", p);
#endif
	    return (p->alloc_data);
	} else if (p->size > (size + sizeof(struct alloc_entry_t))) {
	    pivot = p->next;
	    p->size -= (size + sizeof(struct alloc_entry_t));
	    newmem = (struct alloc_entry_t *)
		((UW) p + (p->size + sizeof(struct alloc_entry_t)));
	    newmem->size = size;
	    newmem->before = newmem->next = 0;
#ifdef notdef
	    dbg_printf
		("malloc: return 0x%x(p = 0x%x), p->size = %d, p->size + sizeof (struct alloc_entry_t) = %d\n",
		 &(p->alloc_data[p->size + sizeof(struct alloc_entry_t)]),
		 p, p->size, p->size + sizeof(struct alloc_entry_t));
#endif
	    return (newmem->alloc_data);
	}
	p = p->next;
    } while (p != pivot);

    /* 要求を満たす空きエリアが無かったので，新たにメモリを割り当てる */
    alloc_size = pageRoundUp(size + sizeof(struct alloc_entry_t));
    alloc_size = (MEMORY_CLICK < alloc_size) ? alloc_size : MEMORY_CLICK;

    newmem = (struct alloc_entry_t *) get_system_memory(alloc_size);
    if (newmem) {
	newmem->next = newmem->before = NULL;
	newmem->size = alloc_size - sizeof(struct alloc_entry_t);
	free(newmem->alloc_data);
	goto retry;
    }
    return (NULL);
}


VP calloc(UW size, UW nentry)
{
    VP rp;

    rp = malloc(size * nentry);
    if (rp) {
	memset(rp, 0, size * nentry);
    }

    return (rp);
}


void free(VP addr)
{
    struct alloc_entry_t *start_point;
    struct alloc_entry_t *p;
    struct alloc_entry_t *newentry;
    UW nextentry;

    newentry =
	(struct alloc_entry_t *) ((B *) addr -
				  sizeof(struct alloc_entry_t));
    if ((newentry > (struct alloc_entry_t *) last_page)
	|| (newentry < (struct alloc_entry_t *) start_page)) {
	/* free するエントリのアドレスがおかしい */
	dbg_printf("[MALLOC] illegal free pointer %x(%x)\n", addr,
		   newentry);
	return;
    }

    if (&alloc_list == alloc_list.next) {
	/* リストにエントリが無い場合: 単に追加すれば良い */
	alloc_list.next = newentry;
	alloc_list.before = newentry;
	newentry->next = newentry->before = &alloc_list;
	return;
    }

    start_point =
	(struct alloc_entry_t *) (((UW) pivot > (UW) newentry) ?
				  &alloc_list : pivot);

    for (p = start_point;; p = p->next) {
#ifdef notdef
	dbg_printf
	    ("malloc: free: newentry = 0x%x, (p = 0x%x, p->size = %d, p->size + sizeof (struct alloc_entry_t) = %d, p->end = 0x%x, p->next = 0x%x)\n",
	     newentry, p, p->size, p->size + sizeof(struct alloc_entry_t),
	     (UW) p + p->size + sizeof(struct alloc_entry_t), p->next);
#endif

	/* フリーリストを先頭から辿り、リストに挿入するポイントを決める */
	/* フリーリストはアドレス順になっており、p のアドレスが追加する */
	/* 領域の最後よりも大きくなったら、その前に挿入する。 */
	nextentry = ((UW) p) + p->size + sizeof(struct alloc_entry_t);
	if ((nextentry <= (UW) newentry) &&
	    ((p->next < p) ||
	     ((((UW) newentry) + newentry->size +
	       sizeof(struct alloc_entry_t)) <= (UW) p->next))) {

	    if (nextentry == (UW) newentry) {
		/* 現在のエントリと隣り合っている */
#ifdef notdef
		dbg_printf
		    ("p->size = %d, newentry->size = %d, p->size + newentry->size + sizeof (...) = %d(0x%x)\n",
		     p->size, newentry->size,
		     p->size + newentry->size +
		     sizeof(struct alloc_entry_t),
		     p->size + newentry->size +
		     sizeof(struct alloc_entry_t));
#endif

		p->size += (newentry->size + sizeof(struct alloc_entry_t));
		if (((UW) p + p->size + sizeof(struct alloc_entry_t)) ==
		    (UW) p->next) {
		    /* newentry を追加した結果次との間が埋まった */
		    p->size +=
			(p->next->size + sizeof(struct alloc_entry_t));
		    p->next->next->before = p;
		    if (pivot == p->next)
			pivot = p;
		    p->next = p->next->next;
		}
		return;
	    } else if ((((UW) newentry) + newentry->size +
			sizeof(struct alloc_entry_t)) == (UW) (p->next)) {
		/* 次のエントリと隣り合っている */
		newentry->size +=
		    (p->next->size + sizeof(struct alloc_entry_t));
		newentry->before = p;
		newentry->next = p->next->next;
		if (pivot == p->next)
		    pivot = newentry;
		p->next = newentry;
		newentry->next->before = newentry;
		return;
	    } else {
		/* 前後のエントリとはどちらとも隣り合っていない -
		 * 単にリストにつなげる
		 */
		newentry->before = p;
		newentry->next = p->next;
		p->next = newentry;
		newentry->next->before = newentry;
		return;
	    }
	}
	if (p == start_point->before)
	    break;
    }				/* for */

#if 1
    /* 該当する部分がなかった */
    dbg_printf("[MALLOC] cannot found insert entry in free list.\n");
#endif
}


static VP get_system_memory(UW size)
{
    ID mytid;
    VP rp;
    ER errno;

    if (get_tid(&mytid) != E_OK) {
	return (NULL);
    }
#ifdef notdef
    dbg_printf("get_system_memory: id = %d, start = 0x%x, size = %d\n",
	       mytid, last_page, size);
#endif
    errno = vmap_reg(mytid, (VP) last_page, size, ACC_USER);
    if (errno) {
#ifdef notdef
	dbg_printf("get_system_memory: can't vmap_reg %d\n", errno);
#endif
	return (NULL);
    }
    rp = (VP) last_page;
    last_page += size;
    return (rp);
}

void print_freelist(void)
{
    struct alloc_entry_t *p;
    int i;

    for (i = 0, p = pivot;; p = p->next, i++) {
	dbg_printf
	    ("[%d]: 0x%x, next = 0x%x, before = 0x%x, size = %d (0x%x)\n",
	     i, p, p->next, p->before, p->size, p->size);
	if (p->next == pivot)
	    break;
	else if (i >= 15)
	    break;
    }
}
