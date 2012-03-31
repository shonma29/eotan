/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* sfs_cache.c - SFS の diskblock の cache を行う
 *
 * $Log: sfs_cache.c,v $
 * Revision 1.1  2000/07/02 04:13:18  naniwa
 * first version
 *
 *
 */
static char rcsid[] =
    "$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/manager/sfs/sfs_cache.c,v 1.1 2000/07/02 04:13:18 naniwa Exp $";

#include "../posix.h"
#include "sfs_func.h"

#define USE_MALLOC

#define HASH_SIZE	30
#ifdef USE_MALLOC
static int CACHE_SIZE = 2048;
#else
#define CACHE_SIZE  (200)
#endif

typedef struct {
    short int dirty;
    short int lru_next, lru_prev;
    short int hash_next, hash_prev;
    W fd;
    W blockno;
    B buf[SFS_BLOCK_SIZE];
} SFS_BLOCK_CACHE;

#ifdef USE_MALLOC
static SFS_BLOCK_CACHE *cache_data;
#else
static SFS_BLOCK_CACHE cache_data[CACHE_SIZE];
#endif
static short int cache_head;
static short int hash_table[HASH_SIZE + 1];

void sfs_purge_cache(void)
{
    int i;

    for (i = 0; i < CACHE_SIZE; ++i) {
	bzero((B *) (&cache_data[i]), sizeof(SFS_BLOCK_CACHE));
	cache_data[i].lru_next = i + 1;
	cache_data[i].lru_prev = i - 1;
	cache_data[i].hash_next = -1;
	cache_data[i].hash_prev = -1;
	cache_data[i].blockno = -1;
    }
    cache_head = 0;
    cache_data[CACHE_SIZE - 1].lru_next = cache_head;
    cache_data[cache_head].lru_prev = CACHE_SIZE - 1;
    for (i = 0; i <= HASH_SIZE; ++i) {
	hash_table[i] = -1;
    }
    hash_table[0] = 0;
}

void sfs_init_cache(void)
{
#ifdef USE_MALLOC
    CACHE_SIZE = ROUNDUP(CACHE_SIZE * sizeof(SFS_BLOCK_CACHE), PAGE_SIZE)
	- 12;
    /* 12 は ITRON/kernlib/malloc.c の alloc_entry_t から来ている */
    CACHE_SIZE /= sizeof(SFS_BLOCK_CACHE);
    cache_data =
	(SFS_BLOCK_CACHE *) malloc(sizeof(SFS_BLOCK_CACHE) * (CACHE_SIZE));
#endif

    sfs_purge_cache();
#ifdef notdef
    sfs_init_bitmap_cache();
#endif
}

void sfs_get_cache(W fd, W blockno, W * cn, B ** ptr)
{
    int i, hn;
    SFS_BLOCK_CACHE *cp;

    if (blockno < 0) {
#ifdef notdef
	dbg_printf("[SFS] alloc cache %d %d\n", blockno, cache_head);
#endif
	*cn = -1;
	*ptr = NULL;
	return;
    }
    hn = (blockno % HASH_SIZE) + 1;
    for (i = hash_table[hn]; i >= 0; i = cache_data[i].hash_next) {
	if ((cache_data[i].fd == fd) && (cache_data[i].blockno == blockno))
	    break;
    }
    if (i < 0) {
#ifdef notdef
	dbg_printf("[SFS] alloc cache %d %d\n", blockno, cache_head);
#endif
	*cn = cache_head;
	if (cache_head == -1) {
	    printk("[SFS] WARNING: no free cache\n");
	    *ptr = NULL;
	    return;
	}

	cp = &cache_data[cache_head];

#ifdef notdef
	dbg_printf
	  ("[SFS] cache over write no=%x:%d to %x:%d, cn=%d/%d dirty = %d\n",
	   cp->fd, cp->blockno, fd, blockno, cache_head, CACHE_SIZE,
	   cp->dirty);
#endif
	if (cp->dirty) {
	    sfs_write_block(cp->fd, cp->blockno, SFS_BLOCK_SIZE, cp->buf);
	    cp->dirty = 0;
	}
	/* remove from hash chain */
	if (cp->blockno >= 0) {
	    /* 既に hash に登録されている場合 */
	    int hn2;
	    hn2 = (cp->blockno % HASH_SIZE) + 1;
#ifdef notdef
	    dbg_printf("[SFS] change %d: %x %d %x %d\n",
		   cache_head, cp->fd, cp->blockno, fd, blockno);
#endif
	    if (cp->hash_prev == -1) {
#ifdef notdef
		if (cache_head != hash_table[hn2])
		    printk("[SFS]: hash_head error\n");
#endif
		hash_table[hn2] = cp->hash_next;
		if (cp->hash_next != -1) {
		    cache_data[cp->hash_next].hash_prev = -1;
		}
	    } else {
		cache_data[cp->hash_prev].hash_next = cp->hash_next;
		if (cp->hash_next != -1) {
		    cache_data[cp->hash_next].hash_prev = cp->hash_prev;
		}
	    }
	}
	bzero(cp->buf, SFS_BLOCK_SIZE);

	sfs_read_block(fd, blockno, SFS_BLOCK_SIZE, cp->buf);
	cp->fd = fd;
	cp->blockno = blockno;
	*ptr = cp->buf;

	/* add to hash chain */
	cp->hash_next = hash_table[hn];
	cp->hash_prev = -1;
	if (hash_table[hn] != -1)
	    cache_data[hash_table[hn]].hash_prev = cache_head;
	hash_table[hn] = cache_head;

	/* remove from lru chain */
#ifdef notdef
	if (cache_head == cp->lru_next) {
	    dbg_printf("[SFS] cache_head make loop!!\n");
	}
#endif
	if (cache_head == cp->lru_next) {
	    cache_head = -1;
	} else {
	    cache_head = cp->lru_next;
	}
	cache_data[cp->lru_next].lru_prev = cp->lru_prev;
	cache_data[cp->lru_prev].lru_next = cp->lru_next;
	cp->lru_prev = (-1);
	cp->lru_next = (-1);
    } else {
#ifdef notdef
      dbg_printf("[SFS] find cache %x %d %d (%d)\n",
	     fd, blockno, i, cache_data[i].dirty);
#endif
	*cn = i;
	*ptr = cache_data[i].buf;
	cp = &cache_data[i];
	if (cp->lru_next != -1) {
	    if (cache_head == i) {
		if (cache_head == cp->lru_next) {
		    cache_head = (-1);
		} else {
		    cache_head = cp->lru_next;
		}
	    }
	    cache_data[cp->lru_next].lru_prev = cp->lru_prev;
	    cache_data[cp->lru_prev].lru_next = cp->lru_next;
	    cp->lru_next = (-1);
	    cp->lru_prev = (-1);
	}
    }
}

void sfs_check_cache(W fd, W blockno, W * cn)
{
    int i, hn;
    SFS_BLOCK_CACHE *cp;

    if (blockno < 0) {
	*cn = -1;
	return;
    }
    hn = (blockno % HASH_SIZE) + 1;
    for (i = hash_table[hn]; i >= 0; i = cache_data[i].hash_next) {
	if ((cache_data[i].fd == fd) && (cache_data[i].blockno == blockno))
	    break;
    }
    if (i < 0) {
	*cn = -1;
    } else {
	*cn = i;
	cp = &cache_data[i];
	if (cp->lru_next != -1) {
	    if (cache_head == i) {
		if (cache_head == cp->lru_next) {
		    cache_head = (-1);
		} else {
		    cache_head = cp->lru_next;
		}
	    }
	    cache_data[cp->lru_next].lru_prev = cp->lru_prev;
	    cache_data[cp->lru_prev].lru_next = cp->lru_next;
	    cp->lru_next = (-1);
	    cp->lru_prev = (-1);
	}
    }
}

/*
 * 引数の dirty が負の場合は cache の dirty flag を強制的に 0 にする．
 */
void sfs_put_cache(W cn, W dirty)
{
    SFS_BLOCK_CACHE *cp;

    if (cn < 0) {
	printk("[SFS] WARNING: negative cache number\n");
	return;
    }

    cp = &cache_data[cn];
    if (dirty > 0) {
	cp->dirty = dirty;
    } else if (dirty < 0) {
#ifdef notdef
	if (cp->dirty) {
	    printk("[SFS] discard cache %d %d", cp->blockno, cn);
	}
#endif
	cp->dirty = 0;
    }

    /* append to lru chain */
    if (cp->lru_next != -1) {
#if 1
	printk("[SFS] block already back!! %d next %d\n", cn,
	       cp->lru_next);
#endif
    } else {
	if (cache_head == -1) {
	    cache_head = cn;
	    cp->lru_prev = cn;
	    cp->lru_next = cn;
	} else {
	    cp->lru_prev = cache_data[cache_head].lru_prev;
	    cp->lru_next = cache_head;
	    cache_data[cp->lru_prev].lru_next = cn;
	    cache_data[cache_head].lru_prev = cn;
	}
    }
}

W sfs_sync_cache(W fd, W umflag)
{
    int i;
    SFS_BLOCK_CACHE *cp;
    W errno;

    for (i = 0; i < CACHE_SIZE; ++i) {
	cp = &cache_data[i];
	if (cp->fd == fd) {
	    if (cp->dirty) {
		errno =
		    sfs_write_block(fd, cp->blockno, SFS_BLOCK_SIZE,
				    cp->buf);
		if (errno < 0) {
		    printk("[SFS] sync_cache write error\n");
		    return (errno);
		}
		cp->dirty = 0;
	    }
	    if (umflag) {
		cp->fd = 0;
	    }
	}
    }
    return (EP_OK);
}
