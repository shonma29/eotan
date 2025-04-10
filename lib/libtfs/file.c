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
#include <string.h>
#include <sys/errno.h>
#include "../libserv/libserv.h"
#include "funcs.h"

static int _synchronize_vnode(vnode_t *);


int tfs_open(vfs_t *fs, const ino_t ino, vnode_t *vnode)
{
	struct tfs_inode *inode = cache_get(&(fs->device), ino);
	if (!inode)
		return EIO;

	vnode->fs = fs;
	vnode->index = inode->i_inumber;
	vnode->mode = inode->i_mode;
	vnode->uid = inode->i_uid;
	vnode->gid = inode->i_gid;
	vnode->size = inode->i_size;
	vnode->private = inode;
	vnode->dirty = false;
	vnode->refer_count = 1;
	vnode->lock_count = 0;
	return 0;
}

int tfs_close(vnode_t *vnode)
{
	struct tfs_inode *inode = vnode->private;
	if (!inode)
		return 0;

	int result = _synchronize_vnode(vnode);
	if (result)
		return result;

	if (!cache_release(inode, false))
		return EIO;

	return 0;
}

int tfs_synchronize(vnode_t *vnode)
{
	return ((vnode->private) ? _synchronize_vnode(vnode) : 0);
}

static int _synchronize_vnode(vnode_t *vnode)
{
	if (vnode->dirty) {
		struct tfs_inode *inode = vnode->private;
		inode->i_inumber = vnode->index;
		inode->i_mode = vnode->mode;
		inode->i_uid = vnode->uid;
		inode->i_gid = vnode->gid;
		inode->i_size = vnode->size;

		if (!cache_modify(vnode->private))
			return EIO;

		vnode->dirty = false;
	}

	return 0;
}

//TODO use off_t
int tfs_read(vnode_t *vnode, copier_t *dest, const unsigned int offset,
		const size_t nbytes, size_t *read_size)
{
	int result = 0;
	vfs_t *fs = vnode->fs;
	struct tfs *tfs = (struct tfs *) (fs->private);
	unsigned int index = offset >> tfs->fs_bshift;
	unsigned int skip = offset & mask(tfs->fs_bshift);
	size_t block_size = tfs->fs_bsize;
	struct tfs_inode *inode = vnode->private;
	size_t rest = nbytes;
	for (; rest > 0; index++) {
		blkno_t block_no = tfs_get_block_no(fs, inode, index);
		if (!block_no) {
			result = EIO;
			break;
		}

		char *src = cache_get(&(fs->device), block_no);
		if (!src) {
			result = EIO;
			break;
		}

		size_t len = block_size - skip;
		if (len > rest)
			len = rest;

		result = dest->copy(dest, &src[skip], len);
		if (result)
			break;

		if (!cache_release(src, false)) {
			result = EIO;
			break;
		}

		skip = 0;
		rest -= len;
	}

	if (nbytes) {
		time_get((SYSTIM *) &(inode->i_atime));
		vnode->dirty = true;
	}

	*read_size = nbytes - rest;
	return result;
}

//TODO use off_t
int tfs_write(vnode_t *vnode, copier_t *src, const unsigned int offset,
		const size_t nbytes, size_t *wrote_size)
{
	SYSTIM clock;
	time_get(&clock);

	int result = 0;
	vfs_t *fs = vnode->fs;
	struct tfs *tfs = (struct tfs *) (fs->private);
	unsigned int index = offset >> tfs->fs_bshift;
	unsigned int skip = offset & mask(tfs->fs_bshift);
	size_t block_size = tfs->fs_bsize;
	struct tfs_inode *inode = vnode->private;
	size_t rest = nbytes;
	for (; rest > 0; index++) {
		blkno_t block_no = tfs_get_block_no(fs, inode, index);
		if (!block_no) {
			block_no = tfs_set_block_no(fs, inode, index,
					tfs_allocate_block(fs));
			if (!block_no) {
				result = EIO;
				break;
			}
		}

		char *dest = cache_get(&(fs->device), block_no);
		if (!dest) {
			result = EIO;
			break;
		}

		size_t len = block_size - skip;
		if (len > rest)
			len = rest;

		result = src->copy(&dest[skip], src, len);
		if (result)
			break;

		if (!cache_release(dest, true))
			return EIO;

		skip = 0;
		rest -= len;
	}

	*wrote_size = nbytes - rest;
	if (*wrote_size) {
		size_t tail = offset + *wrote_size;
		if (tail > vnode->size)
			vnode->size = tail;

		vnode->dirty = true;

		SYSTIM *p;
		p = (SYSTIM *) &(inode->i_mtime);
		*p = clock;
		p = (SYSTIM *) &(inode->i_ctime);
		*p = clock;
	}

	return result;
}
