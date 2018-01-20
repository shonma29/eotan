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
#include <stddef.h>
#ifdef HOST_APP
#include "../../include/set/list.h"
#else
#include <set/list.h>
#endif

typedef struct _block_device_t {
	int channel;
	size_t block_size;
	void (*clear)(struct _block_device_t *, void *);
	int (*read)(struct _block_device_t *, void *, const int);
	int (*write)(struct _block_device_t *, void *, const int);
	int (*invalidate)(struct _block_device_t *, const int);
} block_device_t;

typedef struct {
	int (*mount)();
	int (*unmount)();
	int (*sync)();
	int (*statvfs)();
	int (*getdents)();
	int (*lookup)();
	int (*link)();
	int (*unlink)();
	int (*mkdir)();
	int (*rmdir)();
	int (*stat)();
	int (*wstat)();
	int (*permit)();
	int (*create)();
	int (*close)();
	int (*read)();
	int (*write)();
} vfs_operation_t;

typedef struct _vfs_t {
	list_t bros;
	block_device_t device;
	void *private;
	struct _vnode_t *root;
	struct _vnode_t *origin;
	list_t vnodes;
	vfs_operation_t operations;
} vfs_t;

typedef struct _vnode_t {
	list_t bros;
	vfs_t *fs;
	unsigned int index;
	unsigned int mode;
	size_t size;
	size_t nblock;
	void *private;
	bool dirty;
	unsigned int refer_count;
	unsigned int lock_count;
	struct _vnode_t *covered;
	int dev;
} vnode_t;

struct permission {
	uid_t uid;
	gid_t gid;
};

extern void block_initialize(block_device_t *);

extern int cache_initialize(void);
extern void *cache_create(block_device_t *, const unsigned int);
extern void *cache_get(block_device_t *, const unsigned int);
extern bool cache_modify(const void *);
extern bool cache_release(const void *, const bool);
extern bool cache_invalidate(block_device_t *, const unsigned int);
extern int cache_synchronize(block_device_t *, const bool);

#ifndef HOST_APP
extern int vnodes_initialize(void *(*)(void), void (*)(void *),	const size_t);
extern vnode_t *vnodes_create(void);
extern int vnodes_append(vnode_t *);
extern int vnodes_remove(vnode_t *);
extern vnode_t *vnodes_find(const vfs_t *, const int);
#endif

extern int vfs_lookup(vnode_t *, const char *, const int,
		const struct permission *, vnode_t **);

#endif
