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
#include "../libserv/libserv.h"
#include "funcs.h"


int tfs_stat(vnode_t *vnode, struct stat *st)
{
	st->st_dev = vnode->fs->device.channel;
	st->st_ino = vnode->index;
	st->st_mode = vnode->mode;
	st->st_nlink = 1;
	st->st_uid = vnode->uid;
	st->st_gid = vnode->gid;
	st->st_rdev = 0;
	st->st_size = vnode->size;

	struct tfs_inode *inode = vnode->private;
	st->st_atime = inode->i_atime;
	st->st_mtime = inode->i_mtime;
	st->st_ctime = inode->i_ctime;

	struct tfs *tfs = (struct tfs *) (vnode->fs->private);
	st->st_blksize = tfs->fs_bsize;
	st->st_blocks = (st->st_size + tfs->fs_bsize - 1) >> tfs->fs_bshift;

	return 0;
}

int tfs_wstat(vnode_t *vnode, const struct stat *st)
{
	bool modified = false;
	bool truncated = false;

	if (st->st_mode != (unsigned int) (-1)) {
		vnode->mode = (vnode->mode & S_IFMT) | st->st_mode;
		modified = true;
	}

	if (st->st_gid != (unsigned int) (-1)) {
		vnode->gid = st->st_gid;
		modified = true;
	}

	if (st->st_size != -1) {
		int error_no = tfs_shorten(vnode, 0);
		if (error_no)
			return error_no;

		modified = true;
		truncated = true;
	}

	if (modified) {
		struct tfs_inode *buf = vnode->private;
		SYSTIM *p;
		SYSTIM clock;
		time_get(&clock);

		if (truncated) {
			p = (SYSTIM *) &(buf->i_mtime);
			*p = clock;
		}

		p = (SYSTIM *) &(buf->i_ctime);
		*p = clock;
		vnode->dirty = true;
	}

	return 0;
}
