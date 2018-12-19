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
#ifndef _FS_VFSFUNCS_H_
#define _FS_VFSFUNCS_H_

#include <sys/stat.h>
#include "vfs.h"

static inline int vfs_sync(vnode_t *ip)
{
	return ip->fs->operations.sync(ip);
}

//TODO use off_t
static inline int fs_read(vnode_t *ip, void *buf, const int offset,
		const size_t len, size_t *rlength)
{
	return ((ip->mode & S_IFMT) == S_IFDIR)?
		ip->fs->operations.getdents(ip, buf, offset, len, rlength)
		:vfs_read(ip, buf, offset, len, rlength);
}
//TODO use off_t
static inline int vfs_write(vnode_t *ip, const void *buf, const int offset,
		const size_t len, size_t *rlength)
{
	return ip->fs->operations.write(ip, buf, offset, len, rlength);
}

#endif
