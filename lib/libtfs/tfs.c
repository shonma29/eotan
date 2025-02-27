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
#include <sys/errno.h>
#include "funcs.h"

static int tfs_mount(vfs_t *, vnode_t *);
static int tfs_unmount(vfs_t *);

vfs_operation_t vfs_fsops = {
	tfs_mount,
	tfs_unmount,
	tfs_getdents,
	tfs_walk,
	tfs_remove,
	tfs_stat,
	tfs_wstat,
	tfs_create,
	tfs_close,
	tfs_read,
	tfs_write,
	tfs_synchronize
};


static int tfs_mount(vfs_t *fs, vnode_t *root)
{
	struct tfs *tfs = cache_get(&(fs->device), TFS_SUPER_BLOCK_NO);
	if (!tfs)
		return EIO;

	if ((tfs->fs_magic != TFS_MAGIC)
			|| (tfs->fs_bsize != fs->device.block_size)) {
		cache_release(fs->private, false);
		return EINVAL;
	}

	fs->private = tfs;

	int error_no = tfs_open(fs, tfs->fs_dblkno, root);
	if (error_no)
		cache_release(fs->private, false);

	return error_no;
}

static int tfs_unmount(vfs_t *fs)
{
	return (cache_release(fs->private, false) ? 0 : EIO);
}
