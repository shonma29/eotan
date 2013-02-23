#include "errno.h"
#include "sfs.h"
#include "boot.h"
#include "config.h"
#include "memory.h"
#include <sys/types.h>
#include <elf.h>

#define BLOCK_SIZE      1024

#define ALIGN(x,al)	((((int)x) + (al) - 1) & ~(al - 1))
#define PAGESIZE	4096

static int load_module (struct sfs_inode *ip, int offset,
    struct module_info *info)
{
  char tmp[BLOCK_SIZE];
  Elf32_Ehdr *eHdr = (Elf32_Ehdr*)tmp;
  Elf32_Phdr *pHdr;
  int i;
  int destOffset;
  int first;

  read_file(ip, offset, sizeof(tmp), tmp); 

  if (!isValidModule(eHdr)) {
    boot_printf ("not ELF!\n");
    return E_SYS;
  }

  info->entry = eHdr->e_entry;
  info->mem_length = 0;
  pHdr = (Elf32_Phdr*)(&tmp[eHdr->e_phoff]);
  destOffset = 0;
  first = 1;

  for (i = 0; i < eHdr->e_phnum; pHdr++, i++) {
    int j;
    char *p;

    if (pHdr->p_type != PT_LOAD)	continue;

    if (first) {
      first = 0;
      destOffset = pHdr->p_paddr;
    }

    p = (char*)(info->paddr + pHdr->p_paddr - destOffset);

    if (pHdr->p_filesz > 0) {
      if (read_file(ip, offset + pHdr->p_offset, pHdr->p_filesz, p) == -1) {
        return E_IO;
      }
    }
#ifdef DEBUG
    boot_printf ("offset=%x, filesz=%x, memsz=%x, vaddr=%x, paddr=%x\n",
        offset + pHdr->p_offset, pHdr->p_filesz,
        pHdr->p_memsz, pHdr->p_vaddr, p);
#endif
    p += pHdr->p_filesz;

    /* padding */
    for (j = pHdr->p_memsz - pHdr->p_filesz; j > 0; j--) {
      *p++ = 0;
    }

    info->mem_length = ALIGN(p - info->paddr, PAGESIZE);
  }
#ifdef DEBUG
  boot_printf ("[%x][%x][%x][%x]\n",
      ((char*)info->paddr)[0] & 0xff,
      ((char*)info->paddr)[1] & 0xff,
      ((char*)info->paddr)[2] & 0xff,
      ((char*)info->paddr)[3] & 0xff);
#endif
  boot_printf ("Module: exec size = %x, memsz = %x, entry = %x\n",
	       eHdr->e_shoff,
	       info->mem_length,
	       info->entry);
  return E_OK;
}


static int multi_boot (struct sfs_inode *ip, int rootfs)
{
  int		i;
  int		offset;
  struct boot_header	*info;
  void  	(*entry)();
  int		errno;
  extern UWORD	ticks;

  boot_printf ("Multiple module boot.\n");
  info = (struct boot_header *)MODULE_TABLE;

  errno = read_file(ip, 0, BLOCK_SIZE, (char *)info);
  if (errno) {
    boot_printf ("Couldn't read OS file.\n");
    return (E_SYS);
  }

  info->machine.base_mem = base_mem;
  info->machine.ext_mem = ext_mem;
  info->machine.real_mem = real_mem;
  
  if (rootfs & 0x010000) {
    /* HD */
    info->machine.rootfs = 0x80010000 | ((rootfs & 0xff) + 1);
  }
  else {
    /* FD */
    info->machine.rootfs = 0x80000000 | (rootfs & 0xff);
  }
 
  boot_printf ("Module %d\n", info->count);
  offset = BLOCK_SIZE;	                      /* 最初のモジュールが入っているオフセット(バイト) */

  for (i = 0; i < info->count; i++) {
    boot_printf ("[%d] ", i);
    
    if (load_module (ip, offset, &(info->modules[i])) != E_OK) {
      boot_printf ("Can't load module(s)...abort.\n");
      return (E_SYS);
    }

    offset += info->modules[i].length;
  }
  
  entry = (void (*)())(info->modules[0].entry);
  boot_printf ("exec_info->a_entry = 0x%x\n", entry);
  
  (*entry)();

  return E_OK;
}

int
boot_btron (int no)
{
  int devid = 0;
  struct sfs_inode	os_ip;
  char			buf[BLOCK_SIZE];
  char			*btron_os = BOOT_PATH;
  
  devid = 0x000000 | (no << 8);

  boot_printf ("devid = 0x%x (device = %d, partition = %d)\n", 
	       devid, (devid >> 8) & 0xff, (devid+1) & 0xff);

  if(mount_sfs(devid) == -1) {
    boot_printf("mount failure\n");
    return (E_PAR);
  }
 
  if(lookup_file(btron_os, &os_ip) == -1) {
    boot_printf ("cannot find OS (\"%s\")\n", btron_os);
    return (E_SYS);
  }

  if(read_file (&os_ip, 0, 2, buf) == -1) {
    boot_printf ("cannot read OS file.\n");
    return (E_SYS);
  }

  if (buf[0] == 0x01) {
    boot_printf ("read multiple module.\n");
    multi_boot(&os_ip, devid);
    boot_printf ("boot end\n");
  }
  else {
    boot_printf ("Unknown file type (0x%x)\n", buf[0]);
    return (E_SYS);
  }

  return (E_OK);
}

