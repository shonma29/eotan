/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/*************************************************************************
 *
 *
 */

#ifndef __PAGE_H__
#define __PAGE_H__	1

#include <itron/types.h>
#include "../../kernel/mpu/mpu.h"

#define DIR_SIZE	(PAGE_SIZE * PAGE_SIZE)


void	init_vm (void);
int	map_vm (UW raddr, UW vaddr, UW size);
I386_PAGE_ENTRY *get_page_entry (unsigned long addr);


#endif /* __PAGE_H__ */
