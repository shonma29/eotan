#include "fd.h"
#include "macros.h"
#include "file.h"
#include "page.h"
#include "types.h"
#include "errno.h"
#include "sfs.h"
#include "asm.h"
#include "lib.h"
#include "dma.h"
#include "config.h"
#include "keyboard.h"
#include "bootos.h"

#define TRAD_OPERATOR(name, f)  static int (f) (char* argv[]);
#define HOOK_OPERATOR(name, f)  static int (f##_func) (char* argv[]);
#define EXTN_OPERATOR(name, f)  extern int (f) (char* argv[]);

#include "commands.h"

#undef EXTN_OPERATOR
#undef HOOK_OPERATOR
#undef TRAD_OPERATOR

struct func_entry
{
  char	name[15];
  FUNCP	func;
};

struct func_entry func_table[] =
{
#define EXTN_OPERATOR(name, f) { name, f, },
#define HOOK_OPERATOR(name, f) { name, f##_func, },
#define TRAD_OPERATOR(name, f) { name, f, },
#include "commands.h"
#undef TRAD_OPERATOR
#undef HOOK_OPERATOR
#undef EXTN_OPERATOR
};

static char	**setargs (char *line, char **args);

/**********************************************************************
 *
 */
static char **
setargs (char *line, char **args)
{
  int	i;

  for (i = 0; i < 10 - 1; i++)
    {
      args[i] = line;
      if (*line == '\0')
	goto escape;
      while (*line != '\0')
	{
	  if (*line == ' ')
	    {
	      *line++ = '\0';
	      break;
	    }
	  line++;
	}
      while (*line == ' ')	/* 空白の取り除き */
	{
	  line++;
	}
    }
escape:
  args[i] = NULL;
  return (args);
}

/**********************************************************************
 *
 */
int
evaluate (char *line)
{
  int i;
  char *args[10];		/* いい加減に決めた値 */

  bzero ((void *)args, sizeof (char *) * 10);
  setargs(line, args);

  for (i = 0; i < TABLE_SIZE(func_table); i++)
    {
      if (strncmp (args[0], func_table[i].name, 15) == 0)
	{
	  func_table[i].func (args);
	  break;
	}
    
    }
  if (i >= TABLE_SIZE(func_table))
    {
      boot_printf ("boot: %s: command not found\n", args[0]);
    }

  return E_OK;
}


/**********************************************************************
 * help --- 
 */
static int
help_func (char *argv[])
{
  int	i;
  
  boot_printf ("help:\n");
  boot_printf ("COMMAND:\n");
  for (i = 0; i < TABLE_SIZE(func_table); i++)
    {
      char disp[21];
      int funcname_len;
      strcpy(disp, "                    ");
      funcname_len = strlen(func_table[i].name);
      if (funcname_len > 15)
	{
	  funcname_len = 15;
	}
      strncpy(disp, func_table[i].name, funcname_len);
      boot_printf ("%s", disp);
      if (i % 3 == 2)
	{
	  boot_printf("\n");
	}
    }
  if (i % 3 != 0)
    {
      boot_printf("\n");
    }

  return E_OK;
}


/*********************************************************************
 *
 *
 */

int
boot_btron (char **av)
{
  int devid = 0;
  struct sfs_inode	os_ip;
  char			buf[BLOCK_SIZE];
  char			*btron_os = BOOT_PATH;
  
  if (av[2] == NULL) {
    boot_printf ("usage: boot hd drive partition [btron file]\n");
    boot_printf ("       boot fd drive [btron file]\n");
    return E_PAR;
  }

  if (strcmp (av[1], "hd") == 0) {
    int pn;
    if (av[3] == NULL) {
      boot_printf ("usage: boot hd drive partition [btron file]\n");
      boot_printf ("       boot fd drive [btron file]\n");
      return E_PAR;
    }
    pn = atoi(av[3]);
    if (pn <= 0) {
      boot_printf ("usage: boot hd drive partition [btron file]\n");
      boot_printf ("partition number should start from 1\n");
      return E_PAR;
    }
    pn = pn - 1;
    devid = 0x010000 | (atoi(av[2]) << 8) | pn;
    if (av[4]) {
      btron_os = av[4];
    }
  }
  else if (strcmp (av[1], "fd") == 0) {
    devid = 0x000000 | (atoi(av[2]) << 8);
    if (av[3]) {
      btron_os = av[3];
    }
  }
  else {
    boot_printf ("unknown device. Valid device is \"fd\" or \"hd\"\n");
    return -1;
  }

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


#define HOOK_OPERATOR(name, f) \
static int f##_func (char *argv[]) \
{ \
  f (); \
  return E_OK; \
}
#define EXTN_OPERATOR(name, f)
#define TRAD_OPERATOR(name, f)

#include "commands.h"

#undef TRAD_OPERATOR
#undef EXTN_OPERATOR
#undef HOOK_OPERATOR

