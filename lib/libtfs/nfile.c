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
#include <fs/tfs.h>
#include <sys/errno.h>
#include "func.h"
#include "../libserv/libserv.h"


//TODO use off_t
int tfs_read(vnode_t *vnode, copier_t *dest, unsigned int offset,
		const size_t nbytes, size_t *read_size)
{
	int result = 0;
	vfs_t *fs = vnode->fs;
	size_t block_size = ((struct tfs *) (fs->private))->fs_bsize;
	unsigned int index = offset / block_size;
	unsigned int skip = offset % block_size;
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
	size_t block_size = ((struct tfs *) (fs->private))->fs_bsize;
	unsigned int index = offset / block_size;
	unsigned int skip = offset % block_size;
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
