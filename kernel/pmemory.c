/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* pmemory.c
 *
 *	物理メモリ管理モジュール
 *
 *	このファイルではメモリをページ単位で管理している。
 *
 */

#include <string.h>
#include "core.h"
#include "memory.h"
#include "func.h"
#include "misc.h"
#include "sync.h"
#include "boot.h"
#include "arch/arch.h"

/* memory_map: 0x80000000 から最大 128 M bytes 分のメモリ領域を管理する
 *  ただし最初の 1M と周辺核を読み込んだ部分については管理しない。
 */

#define BITS	8
static UB memory_map[MAX_MEMORY_MAP_SIZE / BITS];

unsigned int physmem_max;	/* 物理メモリの最大量 (base_mem + ext_mem) */
unsigned int base_mem;		/* 下位にあるメモリのサイズ (PC9801 の場合 640K) */
unsigned int ext_mem;		/* 拡張メモリのサイズ */
static UW free_mem = 0;

static UW memory_map_size;
static UB map_mask[] =
    { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0xFF };
static UW bit_free, byte_free;

#ifdef notdef
main()
{
    W n;

    init_memory();
    printk("alloc mem = %d\n", alloc_memory(10));
    printk("alloc mem = %d\n", alloc_memory(10));
    printk("alloc mem = %d\n", n = alloc_memory(5));
    printk("free mem  = %d\n", free_memory(n, 5));
    printk("alloc mem = %d\n", alloc_memory(20));
}

#endif


static void release_memory(UW paddr, UW length)
{
    UW i, j, k, page, index;

    enter_serialize();

    page = PAGES(length);
    index = paddr / PAGE_SIZE;
    i = (index / BITS);
    j = (index % BITS);
    if (j != 0) {
	for (; (j < BITS) && (page > 0); ++j) {
	    memory_map[i] |= map_mask[j];
	    --page;
	}
	++i;
    }
    if (page) {
	for (k = 0; k < page / BITS; ++k, ++i) {
	    memory_map[i] = map_mask[8];
	}
	for (j = 0; j < page % BITS; ++j) {
	    memory_map[i] |= map_mask[j];
	}
    }

    leave_serialize();
}

/*
 *
 */
void pmem_init(void)
{
    W i, j;
    struct boot_header *boot_info;

    boot_info = (struct boot_header *) MODULE_TABLE;
    physmem_max = boot_info->machine.real_mem;
    base_mem = boot_info->machine.base_mem;
    ext_mem = boot_info->machine.ext_mem;

    memory_map_size = physmem_max / PAGE_SIZE;
    if (memory_map_size > MAX_MEMORY_MAP_SIZE) {
	memory_map_size = MAX_MEMORY_MAP_SIZE;
    }
    memory_map_size /= BITS;

    enter_serialize();

    for (i = 0; i < memory_map_size; i++) {
	memory_map[i] = MEM_FREE;
    }

    /* 最初の RESERVED_PAGES 分 (1M) は、管理しない */
#ifdef notdef
    for (i = 0; i < RESERVED_PAGES; i++) {
	memory_map[i] = MEM_USE;
    }
#else
    release_memory(0, RESERVED_MEMORY);
#endif

#ifdef notdef
    /* そして VRAM の領域も管理しない */
    for (i = (VRAM_MEM_START / PAGE_SIZE); i <= (VRAM_MEM_END / PAGE_SIZE);
	 i++) {
	memory_map[i] = MEM_USE;
    }
#endif				/* notdef */

    /* boot が読み込んだITRON と 周辺核の分の領域をフリーリストから解除する */
#if 0
    printk("release: ");
#endif
    if (boot_info->count == 0) {
	extern unsigned char end;

/* #ifdef DEBUG */
	printk("release_memory: 0x%x, 0x%x\n",
	       0x00020000, VTOR((unsigned int) (&end) - 0x80020000));
/* #endif */
	release_memory(0x0020000,
		       VTOR((unsigned int) (&end) - 0x80020000));
    } else {
	for (i = 0; i < boot_info->count; i++) {
#ifdef DEBUG
	    printk("0x%x(%x) ",
		   boot_info->modules[i].paddr,
		   boot_info->modules[i].mem_length);
#endif				/* DEBUG */
	    release_memory(boot_info->modules[i].paddr,
			   boot_info->modules[i].mem_length);
#if 0
	    printk("\n");
#endif
	}
    }

    free_mem = 0;
    bit_free = 0;
    byte_free = 0;
    for (i = 0; i < memory_map_size; ++i) {
	if (memory_map[i] == MEM_FREE) {
	    if (!byte_free)
		byte_free = i;
	    free_mem += BITS;
	} else if (memory_map[i] != map_mask[8]) {
	    if (!bit_free)
		bit_free = i;
	    for (j = 0; j < BITS; ++j) {
		if (!(memory_map[i] & map_mask[j]))
		    ++free_mem;
	    }
	}
    }

    leave_serialize();

#ifdef notdef
    printk("physmem = %d, base_mem = %d, ext_mem = %d\n", physmem_max,
	   base_mem, ext_mem);
    printk("memory_map = %d(%d), free_mem = %d\n",
	   memory_map_size * BITS, memory_map_size, free_mem);
#endif
}

/* palloc --- 物理メモリを連続してアロケートする。
 *
 * 引数はアロケートするサイズを指定する。単位は、ページ数である。
 */
void *palloc(size_t size)
{
    UW i, j, k, addr;
    W bsize, found;

    if (size <= 0)
	return (NULL);
    if (free_mem < size)
	return (NULL);

    enter_serialize();

    if (size == 1) {
	found = 0;
	j = 0;
	for (i = bit_free; i < memory_map_size; ++i) {
	    if (memory_map[i] != map_mask[8]) {
		for (j = 0; j < BITS; ++j) {
		    if (!(memory_map[i] & map_mask[j])) {
			memory_map[i] |= map_mask[j];
			--free_mem;
			found = 1;
			break;
		    }
		}
	    }
	    if (found) {
		for (k = bit_free; k < memory_map_size; ++k) {
		    if (memory_map[k] != map_mask[8]) {
			bit_free = k;
			break;
		    }
		}
		break;
	    }
	}
	if (found == 0) {
	    leave_serialize();

	    return (NULL);
	}
    } else {
	/* BITS ページ単位で割り付けるられる位置を探す */
	found = 0;
	bsize = ROUNDUP(size, BITS) / BITS;
	for (i = byte_free; i < memory_map_size - bsize + 1; ++i) {
	    if (memory_map[i] == MEM_FREE) {
		for (j = i + 1; ((j - i) < bsize) && (j < memory_map_size);
		     ++j) {
		    if (memory_map[j] != MEM_FREE) {
			break;
		    }
		}
		if ((j - i) >= bsize) {
		    found = 1;
		    break;
		}
	    }
	}
	if (found == 0) {
	    leave_serialize();

	    return (NULL);
	}
	for (j = 0; j < size / BITS; ++j) {
	    if (memory_map[i + j] == MEM_FREE)
		free_mem -= BITS;
	    memory_map[i + j] = map_mask[8];
	}
	for (k = 0; k < size % BITS; ++k) {
	    memory_map[i + j] |= map_mask[k];
	    --free_mem;
	}
	for (k = i + j + 1; k < memory_map_size; ++k) {
	    if (memory_map[k] == MEM_FREE) {
		byte_free = k;
		break;
	    }
	}
	j = 0;
    }

    addr = ((i * BITS + j) * PAGE_SIZE) | MIN_KERNEL;
#if 1
    /* initialize to 0 */
    memset((VP)addr, 0, size * PAGE_SIZE);
#endif

    leave_serialize();

    return ((VP) addr);
}

/*
 *
 */
void pfree(void *p, size_t size)
{
    UW i, j, k, index;

    if (size < 0) {
	/*TODO handle error */
	/* return (E_PAR);*/
	return;
    }
    index = ((UW) p) / PAGE_SIZE;
    if ((ROUNDUP(index + size, BITS) / BITS) >= memory_map_size) {
	/*TODO handle error */
	/* return (E_PAR);*/
	return;
    }

    enter_serialize();

    i = (index / BITS);
    j = (index % BITS);
    if (j != 0) {
	if ((i < bit_free) && (size > 0))
	    bit_free = i;
	for (; (j < BITS) && (size > 0); ++j) {
	    if (memory_map[i] & map_mask[j]) {
		++free_mem;
	    }
	    memory_map[i] &= ~map_mask[j];
	    --size;
	}
	++i;
    }
    if (size) {
	if ((i < byte_free) && (size >= BITS))
	    byte_free = i;
	for (k = 0; k < size / BITS; ++k, ++i) {
	    for (j = 0; j < BITS; ++j) {
		if (memory_map[i] & map_mask[j])
		    ++free_mem;
		memory_map[i] &= ~map_mask[j];
	    }
	}
	for (j = 0; j < size % BITS; ++j) {
	    if (memory_map[i] & map_mask[j])
		++free_mem;
	    memory_map[i] &= ~map_mask[j];
	}
    }

    leave_serialize();

    /*TODO handle error */
    /*return (E_OK);*/
    return;
}

UW pmemfree(void)
{
    return (free_mem);
}
