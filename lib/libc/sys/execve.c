/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_exec.c,v 1.2 1999/11/10 10:39:07 naniwa Exp $  */

#include <string.h>
#include <local.h>
#include <nerve/config.h>
#include "sys.h"

#define STACK_TAIL (LOCAL_ADDR - PAGE_SIZE)

/* exec 
 *
 */
int
execve(char *name, char *argv[], char *envp[])
{
  int stsize = 0, i, argc, envc;
  unsigned int offset;

  if (argv != NULL) {
    for(argc = 0; argv[argc] != 0; ++argc){
      stsize += strlen(argv[argc])+1;
    }
  }
  else
    argc = 0;
  if (envp != NULL) {
    for(envc = 0; envp[envc] != 0; ++envc) {
      stsize += strlen(envp[envc])+1;
    }
  }
  else
    envc = 0;
  stsize = (stsize + sizeof(int) - 1) & ~(sizeof(int) - 1);
  stsize += (argc + 1)*4 + (envc + 1)*4 + sizeof(int) * 3;
  offset = STACK_TAIL - stsize;
  {
    char buf[stsize], *strp;
    int bufc;
    void **vp;
    struct posix_request req;

    vp = (void *) ((unsigned int)buf + sizeof(int) * 3);
    vp[-3] = (void*)argc;
    vp[-2] = (void*)(offset + sizeof(int) * 3);
    vp[-1] = (void*)(offset + sizeof(int) * (argc + 1) + sizeof(int) * 3);
    bufc = (argc + envc + 2) * 4 + sizeof(int) * 3;

    for(i = 0; i < argc; ++i) {
      *vp++ = (void *) (offset + bufc);
      strp = argv[i];
      while((buf[bufc++] = *strp++) != 0);
    }
    *vp++ = 0;
    for(i = 0; i < envc; ++i) {
      *vp++ = (void *) (offset + bufc);
      strp = envp[i];
      while((buf[bufc++] = *strp++) != 0);
    }
    *vp++ = 0;
    
    req.param.par_execve.name = name;
    req.param.par_execve.stackp = buf;
    req.param.par_execve.stsize = stsize;

    return _call_fs(PSC_EXEC, &req);
  }
}
