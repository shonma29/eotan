#ifndef _FS_VFS_H_
#define _FS_VFS_H_
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
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <stddef.h>
#ifdef HOST_APP
#include "../../include/copier.h"
#include "../../include/set/list.h"
#else
#include <copier.h>
#include <set/list.h>
#endif

#define UNMODIFIABLE_MODE_BITS (~(S_IRWXU | S_IRWXG | S_IRWXO))

typedef struct _block_device_t {
	int channel;
	size_t block_size;
	void (*clear)(struct _block_device_t *, void *);
	int (*read)(struct _block_device_t *, void *, const int);
	int (*write)(struct _block_device_t *, void *, const int);
	int (*invalidate)(struct _block_device_t *, const int);
} block_device_t;

typedef struct _vfs_operation_t {
	int (*mount)();
	int (*unmount)();
	int (*sync)();
	int (*statvfs)();
	int (*getdents)();
	int (*walk)();
	int (*remove)();
	int (*mkdir)();
	int (*rmdir)();
	int (*stat)();
	int (*wstat)();
	int (*create)();
	int (*close)();
	int (*read)();
	int (*write)();
} vfs_operation_t;

typedef struct _vfs_t {
	block_device_t device;
	void *private;
	struct _vnode_t *root;
	list_t vnodes;
	vfs_operation_t operations;
} vfs_t;

typedef struct _vnode_t {
	list_t bros;
	vfs_t *fs;
//TODO use ino_t (64bit)
//	ino_t index;
	unsigned int index;
	unsigned int mode;
	uid_t uid;
	gid_t gid;
	size_t size;
	void *private;
	bool dirty;
	unsigned int refer_count;
	unsigned int lock_count;
	struct _vnode_t *parent;
} vnode_t;

struct permission {
	uid_t uid;
	gid_t gid;
};

//TODO use off_t
static inline int vfs_write(vnode_t *vnode, copier_t *src,
		const unsigned int offset, const size_t nbytes,
		size_t *wrote_size)
{
	return vnode->fs->operations.write(vnode, src, offset, nbytes,
			wrote_size);
}

static inline int vfs_stat(vnode_t *vnode, struct stat *st)
{
	return vnode->fs->operations.stat(vnode, st);
}

static inline int vfs_wstat(vnode_t *vnode)
{
	return vnode->fs->operations.wstat(vnode);
}

static inline int vfs_sync(vnode_t *vnode)
{
	return vnode->fs->operations.sync(vnode);
}

extern vfs_operation_t vfs_fsops;

extern int copy_from(void *, void *, const size_t);
extern int copy_to(void *, void *, const size_t);

extern int block_initialize(block_device_t *);

extern int cache_initialize(void);
extern void *cache_create(block_device_t *, const unsigned int);
extern void *cache_get(block_device_t *, const unsigned int);
extern bool cache_modify(const void *);
extern bool cache_release(const void *, const bool);
extern bool cache_invalidate(block_device_t *, const unsigned int);
extern int cache_synchronize(block_device_t *, const bool);

extern int vnodes_initialize(void *(*)(void), void (*)(void *),	const size_t);
extern vnode_t *vnodes_create(vnode_t *);
extern int vnodes_append(vnode_t *);
extern int vnodes_remove(vnode_t *);
extern vnode_t *vnodes_find(const vfs_t *, const int);

extern int vfs_mount(const int, vfs_t *, vnode_t *);
extern int vfs_walk(vnode_t *, char *, const int,
		const struct permission *, vnode_t **);
extern int vfs_open(vnode_t *, const int, struct permission *);
extern int vfs_create(vnode_t *, char *, const int, const mode_t,
		const struct permission *, vnode_t **);
extern int vfs_remove(vnode_t *, const struct permission *);
extern int vfs_permit(const vnode_t *, const struct permission *,
		const unsigned int);
extern int vfs_read(vnode_t *, copier_t *, const unsigned int, const size_t,
		size_t *);

#endif
