#include <itron/types.h>
#include <mpu/config.h>
#include "../../kernel/mpu/mpu.h"
#include "config.h"
#include "errno.h"
#include "page.h"
#include "lib.h"
#include "memory.h"

I386_DIRECTORY_ENTRY *page_dir;
I386_PAGE_ENTRY *page_entry;

static void init_dir_ent(I386_DIRECTORY_ENTRY *dir,
			 UW page_addr, UW entry);

static void init_page_ent(I386_PAGE_ENTRY *page,
			  UW paddr,
			  UW entry, UW vaddr, UW mode);


/**************************************************************************
 * vm_init
 *
 */
void init_vm(void)
{
    page_dir = (I386_DIRECTORY_ENTRY *) PAGE_DIR_ADDR;
    page_entry = (I386_PAGE_ENTRY *) PAGE_ENTRY_ADDR;
    memset((char *) page_dir, 0,
	  MAX_DIRENT * sizeof(I386_DIRECTORY_ENTRY));
    memset((char *) page_entry, 0,
	  MAX_PAGEENT * sizeof(I386_PAGE_ENTRY));

    init_dir_ent(page_dir, PAGE_ENTRY_ADDR,
		 (MIN_MEMORY_SIZE * sizeof(I386_DIRECTORY_ENTRY)) /
		 DIR_SIZE);
    init_dir_ent(&page_dir[512], PAGE_ENTRY_ADDR,
		 (MIN_MEMORY_SIZE * sizeof(I386_DIRECTORY_ENTRY)) /
		 DIR_SIZE);
    init_page_ent(page_entry, 0x0, MIN_MEMORY_SIZE / PAGE_SIZE, 0x80000000,
		  0);
}

/***************************************************************************
 * get_page_entry ---
 */
I386_PAGE_ENTRY *get_page_entry(UW addr)
{
    int dir_entry;
    int page_entry;
    I386_PAGE_ENTRY *pent;

    dir_entry = (addr >> 22) & 0x3ff;
    page_entry = (addr >> 12) & 0x3ff;
    pent =
	(I386_PAGE_ENTRY *) (page_dir[dir_entry].frame_addr << 12);
    return ((I386_PAGE_ENTRY *) &pent[page_entry]);
}

/***************************************************************************
 *
 */
static void
init_dir_ent(I386_DIRECTORY_ENTRY *dir,
	     UW page_addr, UW entry)
{
    int i;

    for (i = 0; i < entry; i++) {
	dir[i].frame_addr = (page_addr + (i * 4096)) / 4096;
	dir[i].user = 0;
	dir[i].zero1 = 0;
	dir[i].dirty = 0;
	dir[i].access = 0;
	dir[i].zero2 = 0;
	dir[i].u_and_s = 0;
	dir[i].read_write = 1;
	dir[i].present = 1;
    }
}

/***************************************************************************
 *
 */
static void
init_page_ent(I386_PAGE_ENTRY *page,
	      UW paddr, UW entry, UW vaddr, UW mode)
{
    int i;

    for (i = 0; i < entry; i++) {
	page[i].frame_addr = (paddr + (i * 4096)) / 4096;
	page[i].user = 0;
	page[i].zero1 = 0;
	page[i].dirty = 0;
	page[i].access = 0;
	page[i].zero2 = 0;
	page[i].u_and_s = mode;
	page[i].read_write = 1;
	page[i].present = 1;
    }
}
