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
#include <stdint.h>
#include <string.h>
#include <fs/nconfig.h>
#include <fs/vfs.h>
#include <mpu/memory.h>
#include <nerve/kcall.h>
#include <set/list.h>
#include <set/slab.h>

static slab_t vnodes_slab;


static inline vnode_t *getINodeParent(const list_t *p) {
	return (vnode_t*)((intptr_t)p - offsetof(vnode_t, bros));
}

int vnodes_initialize(void)
{
	vnodes_slab.unit_size = sizeof(vnode_t);
	vnodes_slab.block_size = PAGE_SIZE;
	vnodes_slab.min_block = 1;
	vnodes_slab.max_block = MAX_VNODE
			/ ((PAGE_SIZE - sizeof(slab_block_t))
					/ sizeof(vnode_t));
	vnodes_slab.palloc = kcall->palloc;
	vnodes_slab.pfree = kcall->pfree;
	slab_create(&vnodes_slab);

	return 0;
}

vnode_t *vnodes_create(void)
{
	vnode_t *ip = slab_alloc(&vnodes_slab);
	if (!ip)
		return NULL;

	memset(ip, 0, sizeof(vnode_t));
	list_initialize(&(ip->bros));
	ip->refer_count = 1;

	return ip;
}

int vnodes_append(vnode_t *ip)
{
	list_push(&(ip->fs->vnodes), &(ip->bros));

	return 0;
}

int vnodes_remove(vnode_t *ip)
{
	ip->refer_count--;

	if (!(ip->refer_count)) {
		if (!(ip->dev)
				&& ip->fs) {
			ip->fs->operations.close(ip);
//			int error_no = ip->fs->operations.close(ip);
//			if (error_no)
//				return error_no;
		}

		list_remove(&(ip->bros));
		slab_free(&vnodes_slab, ip);
	}

	return 0;
}

vnode_t *vnodes_find(const vfs_t *fsp, const int index)
{
	const list_t *vnodes = &(fsp->vnodes);
	
	for (list_t *p = list_next(vnodes); !list_is_edge(vnodes, p);
			p = list_next(p)) {
		vnode_t *ip = getINodeParent(p);

		if (ip->index == index)
			return ip;
	}

	return NULL;
}
