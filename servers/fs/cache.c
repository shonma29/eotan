/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* cache.c - FS の diskblock の cache を行う
 *
 * $Log: sfs_cache.c,v $
 * Revision 1.1  2000/07/02 04:13:18  naniwa
 * first version
 *
 *
 */

#include <string.h>
#include <mpu/memory.h>
#include "fs.h"

#undef USE_MALLOC

#define HASH_SIZE	30
#ifdef USE_MALLOC
//static int CACHE_SIZE = 2048;
static int CACHE_SIZE = 256;
#else
#define CACHE_SIZE  (200)
#endif

typedef struct {
    short int dirty;
    short int lru_next, lru_prev;
    short int hash_next, hash_prev;
    W fd;
    W blockno;
    B buf[BLOCK_SIZE];
} BLOCK_CACHE;

#ifdef USE_MALLOC
static BLOCK_CACHE *cache_data;
#else
static BLOCK_CACHE cache_data[CACHE_SIZE];
#endif
static short int cache_head;
static short int hash_table[HASH_SIZE + 1];

static W read_block (ID device, W blockno, W blocksize, B *buf);
static W write_block (ID device, W blockno, W blocksize, B *buf);

void purge_cache(void)
{
    int i;

    for (i = 0; i < CACHE_SIZE; ++i) {
	memset((B*)(&cache_data[i]), 0, sizeof(BLOCK_CACHE));
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

void init_cache(void)
{
#ifdef USE_MALLOC
    CACHE_SIZE = pageRoundUp(CACHE_SIZE * sizeof(BLOCK_CACHE))
	- 12;
    /* 12 は ITRON/kernlib/malloc.c の alloc_entry_t から来ている */
    CACHE_SIZE /= sizeof(BLOCK_CACHE);
    cache_data =
	(BLOCK_CACHE *) malloc(sizeof(BLOCK_CACHE) * (CACHE_SIZE));
#endif

    purge_cache();
}

void get_cache(W fd, W blockno, W * cn, B ** ptr)
{
    int i, hn;
    BLOCK_CACHE *cp;

    if (blockno < 0) {
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
	*cn = cache_head;
	if (cache_head == -1) {
	    dbg_printf("fs: WARNING: no free cache\n");
	    *ptr = NULL;
	    return;
	}

	cp = &cache_data[cache_head];

	if (cp->dirty) {
	    write_block(cp->fd, cp->blockno, BLOCK_SIZE, cp->buf);
	    cp->dirty = 0;
	}
	/* remove from hash chain */
	if (cp->blockno >= 0) {
	    /* 既に hash に登録されている場合 */
	    int hn2;
	    hn2 = (cp->blockno % HASH_SIZE) + 1;
	    if (cp->hash_prev == -1) {
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
	memset(cp->buf, 0, BLOCK_SIZE);

	read_block(fd, blockno, BLOCK_SIZE, cp->buf);
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

void check_cache(W fd, W blockno, W * cn)
{
    int i, hn;
    BLOCK_CACHE *cp;

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
void put_cache(W cn, W dirty)
{
    BLOCK_CACHE *cp;

    if (cn < 0) {
	dbg_printf("fs: WARNING: negative cache number\n");
	return;
    }

    cp = &cache_data[cn];
    if (dirty > 0) {
	cp->dirty = dirty;
    } else if (dirty < 0) {
	cp->dirty = 0;
    }

    /* append to lru chain */
    if (cp->lru_next != -1) {
	dbg_printf("fs: block already back!! %d next %d\n", cn,
	       cp->lru_next);
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

W sync_cache(W fd, W umflag)
{
    int i;
    BLOCK_CACHE *cp;
    W error_no;

    for (i = 0; i < CACHE_SIZE; ++i) {
	cp = &cache_data[i];
	if (cp->fd == fd) {
	    if (cp->dirty) {
		error_no =
		    write_block(fd, cp->blockno, BLOCK_SIZE,
				    cp->buf);
		if (error_no < 0) {
		    dbg_printf("fs: sync_cache write error\n");
		    return (error_no);
		}
		cp->dirty = 0;
	    }
	    if (umflag) {
		cp->fd = 0;
	    }
	}
    }
    return (EOK);
}

/* read_block - ブロックをひとつ読み込む
 *
 */
static W read_block(ID device, W blockno, W blocksize, B * buf)
{
    W rsize;
    W error_no;

#ifdef FMDEBUG
    dbg_printf
	("fs: read_block: device = %d, blockno = %d, blocksize = %d, buf = 0x%x\n",
	 device, blockno, blocksize, buf);
#endif

    error_no =
	read_device(device, buf, blockno * blocksize, blocksize,
			&rsize);
    if (error_no) {
	return (error_no);
    }

    return (rsize);
}

/* write_block - ブロックをひとつ書き込む
 *
 */
static W write_block(ID device, W blockno, W blocksize, B * buf)
{
    W error_no;
    W rsize;

    error_no =
	write_device(device, buf, blockno * blocksize, blocksize,
			 &rsize);
    if (error_no) {
	return (error_no);
    }

    return (rsize);
}
