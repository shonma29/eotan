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
#include "posix.h"


/* exec 
 *
 */
int
execve(char *name, char *argv[], char *envp[])
{
  int stsize = 0, i, argc, envc;

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
  stsize += (argc + 1)*4 + (envc + 1)*4;
  {
    char buf[stsize], *strp;
    int bufc;
    void **vp;
    struct posix_request req;

    vp = (void *) buf;
    bufc = (argc + envc + 2) * 4;

    for(i = 0; i < argc; ++i) {
      *vp++ = (void *) bufc;
      strp = argv[i];
      while((buf[bufc++] = *strp++) != 0);
    }
    *vp++ = 0;
    for(i = 0; i < envc; ++i) {
      *vp++ = (void *) bufc;
      strp = envp[i];
      while((buf[bufc++] = *strp++) != 0);
    }
    *vp++ = 0;
    
    req.param.par_execve.pathlen = strlen(name);
    req.param.par_execve.name = name;
    req.param.par_execve.stackp = buf;
    req.param.par_execve.stsize = stsize;

    return _call_fs(PSC_EXEC, &req);
  }
}