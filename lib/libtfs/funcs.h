#ifndef _TFS_FUNCS_H_
#define _TFS_FUNCS_H_
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
#include <stddef.h>
#include <fs/vfs.h>
#include <sys/dirent.h>

/* block.c */
extern blkno_t tfs_allocate_block(vfs_t *);
extern int tfs_deallocate_block(vfs_t *, const blkno_t);
extern int tfs_deallocate_1st(vfs_t *, struct tfs_inode *, const unsigned int,
		const unsigned int);
extern blkno_t tfs_get_block_no(vfs_t *, const struct tfs_inode *,
		const unsigned int);
extern blkno_t tfs_set_block_no(vfs_t *, struct tfs_inode *,
		const unsigned int, const blkno_t);
extern int tfs_allocate_inode(vfs_t *, vnode_t *);
extern int tfs_deallocate_inode(vfs_t *, vnode_t *);

/* nfile.c */
//TODO use off_t
extern int tfs_read(vnode_t *, copier_t *, const unsigned int, const size_t,
		size_t *);
extern int tfs_write(vnode_t *, copier_t *, const unsigned int, const size_t,
		size_t *);

/* ndirectory.c */
extern int tfs_getdents(vnode_t *, copier_t *, const int, const size_t,
		size_t *);
extern int tfs_walk(vnode_t *, const char *, vnode_t **);
extern int tfs_mkdir(vnode_t *, const char *, const mode_t mode,
			     struct permission *, vnode_t **);
extern int tfs_append_entry(vnode_t *, const char *, vnode_t *);
extern int tfs_remove(vnode_t *, vnode_t *);

/* stat.c */
extern int tfs_wstat(vnode_t *, const struct stat *);

#endif
