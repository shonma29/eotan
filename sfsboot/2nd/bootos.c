#include "sfs.h"
#include "boot.h"
#include "config.h"
#include "file.h"
#include "a.out.h"
#include "memory.h"

int
load_module (struct sfs_inode *ip, int offset, struct module_info *info)
{
  char	tmp[BLOCK_SIZE];
  struct exec	*exec_info;

  read_file(ip, offset, sizeof (struct exec), tmp); 
  exec_info = (struct exec *)tmp;

  if (N_BADMAG(*exec_info)) {
    boot_printf("This object is not exec format (%d).\n", *exec_info);
    return (E_SYS);
  }
  
  boot_printf ("[%s]\n",info->name);
  boot_printf ("Module: exec type = 0x%x, Text size = %d, Data size = %d\n",
	       N_MAGIC(*exec_info),
	       exec_info->a_text,
	       exec_info->a_data);

  if((N_MAGIC(*exec_info) == 0413) || (N_MAGIC(*exec_info) == NMAGIC)) {
    if(read_file(ip, offset + BLOCK_SIZE, exec_info->a_text + exec_info->a_data, 
		 (char *)info->paddr) == -1)
      return E_IO;
  }
  else {
    boot_printf ("I don't know how to read a.out image.(0x%x)\n", N_MAGIC(*exec_info));
    return E_SYS;
  }

  return E_OK;
}


int
multi_boot (struct sfs_inode *ip, int rootfs)
{
  int		i;
  int		offset;
  struct boot_header	*info;
  void  	(*entry)();
  int		errno;
  extern UWORD	ticks, clock;

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
  entry = (void (*)())(info->modules[0].entry);

  for (i = 0; i < info->count; i++) {
    boot_printf ("[%d] ", i);
    
    if (load_module (ip, offset, &(info->modules[i])) != E_OK) {
      boot_printf ("Can't load module(s)...abort.\n");
      return (E_SYS);
    }

    offset += info->modules[i].length + BLOCK_SIZE;	/* ??? */
    
  }
  
  boot_printf ("exec_info->a_entry = 0x%x\n", entry); 
  
  info->machine.clock = clock + ticks/TICKS;
  (*entry)();

  return E_OK;
}
