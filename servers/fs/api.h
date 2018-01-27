#ifndef _FS_API_H_
#define _FS_API_H_
/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/
#include <core.h>
#include <fs/config.h>
#include <sys/syscall.h>
#include <sys/syslimits.h>

typedef struct {
	struct posix_request packet;
	RDVNO rdvno;
	B buf[NAME_MAX + 1];
} fs_request;

extern int if_chdir(fs_request*);
extern int if_chmod(fs_request*);
extern int if_close(fs_request*);
extern int if_dup2(fs_request*);
extern int if_exec(fs_request*);
extern int if_exit(fs_request*);
extern int if_fork(fs_request*);
extern int if_kill(fs_request*);
extern int if_link(fs_request*);
extern int if_lseek(fs_request*);
extern int if_mkdir(fs_request*);
extern int if_open(fs_request*);
extern int if_read(fs_request*);
extern int if_rmdir(fs_request*);
extern int if_fstat(fs_request*);
extern int if_unlink(fs_request*);
extern int if_waitpid(fs_request*);
extern int if_write(fs_request*);
extern int if_mount(fs_request*);
extern int if_unmount(fs_request*);
extern int if_getdents(fs_request*);

#endif
