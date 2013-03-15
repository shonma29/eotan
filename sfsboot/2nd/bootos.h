#ifndef __BOOTOS_H__
#define __BOOTOS_H__

#include <elf.h>

extern int isValidModule(Elf32_Ehdr *eHdr);

extern int boot_btron(int no);

#endif
