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
#include <fs/tfs.h>
#include <sys/unistd.h>
#include "../libserv/libserv.h"
#include "func.h"
#include "funcs.h"


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
		int result = sfs_i_truncate(vnode, 0);
		if (result)
			return result;

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
