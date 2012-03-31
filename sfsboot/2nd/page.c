#include "types.h"
#include "config.h"
#include "location.h"
#include "errno.h"
#include "page.h"
#include "lib.h"
#include "memory.h"

struct page_directory_entry *page_dir;
struct page_table_entry *page_entry;

static void init_dir_ent(struct page_directory_entry *dir,
			 ULONG page_addr, ULONG entry);

static void init_page_ent(struct page_table_entry *page,
			  ULONG paddr,
			  ULONG entry, ULONG vaddr, ULONG mode);


/**************************************************************************
 * vm_init
 *
 */
void init_vm(void)
{
    page_dir = (struct page_directory_entry *) PAGE_DIR_ADDR;
    page_entry = (struct page_table_entry *) PAGE_ENTRY_ADDR;
    bzero((char *) page_dir,
	  MAX_DIRENT * sizeof(struct page_directory_entry));
    bzero((char *) page_entry,
	  MAX_PAGEENT * sizeof(struct page_table_entry));

#if 0
    init_dir_ent(page_dir, PAGE_ENTRY_ADDR, 1);
    init_dir_ent(&page_dir[512], PAGE_ENTRY_ADDR + PAGE_SIZE * 2, 4);
    init_page_ent(page_entry, 0x0, 1024 * 2, 0, 0);
    init_page_ent(&page_entry[2048], 0x0, 1024 * 2, 0x80000000, 0);
#else
    init_dir_ent(page_dir, PAGE_ENTRY_ADDR,
		 (MIN_MEMORY_SIZE * sizeof(struct page_directory_entry)) /
		 DIR_SIZE);
    init_dir_ent(&page_dir[512], PAGE_ENTRY_ADDR,
		 (MIN_MEMORY_SIZE * sizeof(struct page_directory_entry)) /
		 DIR_SIZE);
    init_page_ent(page_entry, 0x0, MIN_MEMORY_SIZE / PAGE_SIZE, 0x80000000,
		  0);
#endif
}

/***************************************************************************
 * get_page_entry ---
 */
struct page_table_entry *get_page_entry(ULONG addr)
{
    int dir_entry;
    int page_entry;
    struct page_table_entry *pent;

    dir_entry = (addr >> 22) & 0x3ff;
    page_entry = (addr >> 12) & 0x3ff;
    pent =
	(struct page_table_entry *) (page_dir[dir_entry].frame_addr << 12);
    return ((struct page_table_entry *) &pent[page_entry]);
}

/***************************************************************************
 *
 */
static void
init_dir_ent(struct page_directory_entry *dir,
	     ULONG page_addr, ULONG entry)
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
init_page_ent(struct page_table_entry *page,
	      ULONG paddr, ULONG entry, ULONG vaddr, ULONG mode)
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
