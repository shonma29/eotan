#ifndef _FS_FS_H_
#define _FS_FS_H_
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
#include <fs/vfs.h>
#include <set/tree.h>

//TODO use off_t
//TODO move to vfs.h
static inline int fs_read(vnode_t *vnode, copier_t *dest, const int offset,
		const size_t len, size_t *rlength)
{
	return ((vnode->mode & S_IFMT) == S_IFDIR) ?
		vnode->fs->operations.getdents(vnode, dest, offset, len,
				rlength)
		: vfs_read(vnode, dest, offset, len, rlength);
}

extern vnode_t *rootfile;

extern int fs_initialize(void);
extern int fs_mount(const int);

#endif
