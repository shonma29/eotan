#ifndef _UNISTD_H_
#define _UNISTD_H_
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
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/unistd.h>

#define STDIN_FILENO (0)
#define STDOUT_FILENO (1)
#define STDERR_FILENO (2)

extern char **environ;

extern int chdir(char *);
extern int access(const char *, int);
extern void *sbrk(intptr_t);
extern int close(int);
extern int dup2(int, int);
extern int execve(char *name, char *argv[], char *envp[]);
extern void _exit(int status) __attribute__ ((noreturn));
extern int fork(void);
extern char *getcwd(char *buf, int size);
extern gid_t getgid(void);
extern pid_t getpid(void);
extern pid_t getppid(void);
extern uid_t getuid(void);
extern int link(const char *, const char *);
extern off_t lseek(int, off_t, int);
extern int open(const char *path, int oflag, ...);
extern ssize_t read(int, void *, size_t);
extern int rmdir(const char *);
extern unsigned int sleep(unsigned int);
extern int unlink(const char *);
extern int waitpid(pid_t, int *, int);
extern size_t write(int, const void *, size_t);

#endif
