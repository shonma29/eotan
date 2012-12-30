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

extern int chdir(char *path);
extern int access(char *path, int mode);
extern int brk(void *endds);
extern int chmod(char *path, int mode);
extern int chown(char *path, int owner, int group);
extern int close(int fd);
extern int closedir(int fd);
extern int creat(char *path, int mode);
extern int dup(int fd);
extern int dup2(int fd, int fd2);
extern int execve(char *name, char *argv[], char *envp[]);
extern int _exit(int status);
extern int fcntl(int fileid, int cmd, ...);
extern int fork();
/*
extern int fstat(int fd, struct stat *st);
extern int getdents(unsigned int fd, struct dirent *dirp, unsigned int count);
*/
extern int getegid(void);
extern char *getcwd(char *buf, int size);
extern int geteuid(void);
extern int getgid(void);
extern int getgrgid(void);
extern int getpid(void);
extern int getppid(void);
extern int getuid(void);
extern int kill(int pid);
extern int link(char *src, char *dst);
extern int lseek(int fd, int offset, int mode);
extern int misc(int cmd, int len, void *argp);
extern int mkdir(char *path, int mode);
extern int mntroot(int devnum);
extern int mount(char *special_file, char *dir, int rwflag, char *fstype);
extern int open(char *path, int oflag, int mode);
extern int opendir(char *path, int mode);
extern int read(int fd, void *buf, int size);
/*
extern int readdir(int fd, struct dirent *dirp, int count);
*/
extern int remove(char *path);
extern int rename(char *src, char *dst);
extern int rmdir(char *path);
extern int setgid(int gid);
extern int setpgid(int gid);
extern int setuid(int uid);
extern int usleep(int usecond);
extern int sleep(int second);
/*
extern int gettimeofday(struct timeval *tv, struct timezone *tz);
extern int time(time_t *tm);
extern int umask(mode_t mask);
*/
extern int umount(char *special_file);
extern int uname(char *buf);
extern int unlink(char *path);
/*
extern int utime(char *path, struct utimbuf *buf);
extern int waitpid(pid_t pid, int *status, int option);
*/
extern int write(int fd, void *buf, int length);

#endif
