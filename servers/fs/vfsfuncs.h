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

#include <fs/vfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "api.h"
#include "vfs.h"

struct file {
	vnode_t *f_vnode;
	uint_fast32_t f_flag;
	uint_fast32_t f_count;
	off_t f_offset;
};

static inline pid_t unpack_pid(const fs_request *req)
{
	return (req->packet.process_id & 0xffff);
}

static inline int unpack_tid(const fs_request *req)
{
	return ((req->packet.process_id >> 16) & 0xffff);
}

static inline int vfs_sync(vnode_t *vnode)
{
	return vnode->fs->operations.sync(vnode);
}

//TODO use off_t
static inline int fs_read(vnode_t *vnode, void *buf, const int offset,
		const size_t len, size_t *rlength)
{
	return ((vnode->mode & S_IFMT) == S_IFDIR)?
		vnode->fs->operations.getdents(vnode, buf, offset, len, rlength)
		:vfs_read(vnode, buf, offset, len, rlength);
}
//TODO use off_t
static inline int vfs_write(vnode_t *vnode, const void *buf, const int offset,
		const size_t len, size_t *rlength)
{
	return vnode->fs->operations.write(vnode, buf, offset, len, rlength);
}

static inline int vfs_stat(vnode_t *vnode, struct stat *st)
{
	return vnode->fs->operations.stat(vnode, st);
}

static inline int vfs_wstat(vnode_t *vnode)
{
	return vnode->fs->operations.wstat(vnode);
}

extern int session_get_path(vnode_t **, const pid_t, const int,
		unsigned char *, unsigned char *);

#endif
