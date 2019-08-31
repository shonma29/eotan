/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_exec.c,v 1.2 1999/11/10 10:39:07 naniwa Exp $  */

#include <core.h>
#include <errno.h>
#include <services.h>
#include <string.h>
#include <sys/syscall.h>

#define STACK_TAIL (LOCAL_ADDR - PAGE_SIZE)

/* exec 
 *
 */
int
execve(const char *name, char *const argv[], char *const envp[])
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

    vp = (void *) ((unsigned int)buf + sizeof(int) * 3);
    vp[-3] = (void*)argc;
    vp[-2] = (void*)(offset + sizeof(int) * 3);
    vp[-1] = (void*)(offset + sizeof(int) * (argc + 1) + sizeof(int) * 3);
    bufc = (argc + envc + 2) * 4 + sizeof(int) * 3;

    for(i = 0; i < argc; ++i) {
      *vp = (void *) (offset + bufc);
      vp++;
      strp = argv[i];
      while((buf[bufc++] = *strp++) != 0);
    }
    *vp = 0;
    vp++;
    for(i = 0; i < envc; ++i) {
      *vp = (void *) (offset + bufc);
      vp++;
      strp = envp[i];
      while((buf[bufc++] = *strp++) != 0);
    }
    *vp = 0;
    vp++;
    
	sys_args_t args = {
		syscall_exec,
		(int)name,
		(int)buf,
		stsize

	};

	ER_UINT reply_size = cal_por(PORT_MM, 0xffffffff, &args, sizeof(args));
	sys_reply_t *reply = (sys_reply_t*)&args;
	if (reply_size == sizeof(*reply)) {
		if (reply->result == -1)
			_set_local_errno(reply->data[0]);

		return reply->result;
	} else {
		_set_local_errno(ECONNREFUSED);
		return (-1);
	}
  }
}
