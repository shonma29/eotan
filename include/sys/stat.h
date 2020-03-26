#ifndef _SYS_STAT_H_
#define _SYS_STAT_H_
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
#include <sys/types.h>

struct stat {
	dev_t st_dev;
	ino_t st_ino;
	mode_t st_mode;
	nlink_t st_nlink;
	uid_t st_uid;
	gid_t st_gid;
	dev_t st_rdev;
	off_t st_size;
	time_t st_atime;
	time_t st_mtime;
	time_t st_ctime;
	blksize_t st_blksize;
	blkcnt_t st_blocks;
};

#define S_IFMT (0170000)
#define S_IFSOCK (0140000)
#define S_IFLNK (0120000)
#define S_IFREG (0100000)
#define S_IFBLK (0060000)
#define S_IFDIR (0040000)
#define S_IFCHR (0020000)
#define S_IFIFO (0010000)

#define S_ISUID (0004000)
#define S_ISGID (0002000)
#define S_ISVTX (0001000)

#define S_IRWXU (0700)
#define S_IRUSR (0400)
#define S_IWUSR (0200)
#define S_IXUSR (0100)
#define S_IRWXG (0070)
#define S_IRGRP (0040)
#define S_IWGRP (0020)
#define S_IXGRP (0010)
#define S_IRWXO (0007)
#define S_IROTH (0004)
#define S_IWOTH (0002)
#define S_IXOTH (0001)

#define DMDIR 0x80000000

extern int chmod(const char *, mode_t);
extern int stat(const char *, struct stat *);
extern int fstat(int, struct stat *);
extern int mkdir(const char *, mode_t);

#endif
