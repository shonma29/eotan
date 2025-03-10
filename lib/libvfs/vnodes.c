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
#include <fs/config.h>
#include <fs/vfs.h>
#include <mpu/memory.h>
#include <nerve/kcall.h>
#include <set/list.h>
#include <set/slab.h>

static slab_t vnodes_slab;


static inline vnode_t *getINodeParent(const list_t *p)
{
	return ((vnode_t *) ((uintptr_t) p - offsetof(vnode_t, bros)));
}

int vnodes_initialize(void *(*palloc)(void), void (*pfree)(void *),
		const size_t max_vnode)
{
	vnodes_slab.unit_size = sizeof(vnode_t);
	vnodes_slab.block_size = PAGE_SIZE;
	vnodes_slab.min_block = 1;
	vnodes_slab.max_block = slab_max_block(max_vnode, PAGE_SIZE,
			sizeof(vnode_t));
	vnodes_slab.palloc = palloc;
	vnodes_slab.pfree = pfree;
	slab_create(&vnodes_slab);
	return 0;
}

vnode_t *vnodes_create(vnode_t *parent)
{
	vnode_t *vnode = slab_alloc(&vnodes_slab);
	if (!vnode)
		return NULL;

	memset(vnode, 0, sizeof(vnode_t));
	list_initialize(&(vnode->bros));
	vnode->refer_count = 1;

	if (parent) {
		vnode->parent = parent;
		parent->refer_count++;
	}

	return vnode;
}

int vnodes_append(vnode_t *vnode)
{
	list_push(&(vnode->fs->vnodes), &(vnode->bros));
	return 0;
}

int vnodes_remove(vnode_t *vnode)
{
	vnode->refer_count--;
	if (!(vnode->refer_count)) {
		if (vnode->fs) {
			int error_no = vnode->fs->operations.close(vnode);
			if (error_no)
				return error_no;
		}

		if (vnode->parent) {
			// discard error
			vnodes_remove(vnode->parent);
			vnode->parent = NULL;
		}

		list_remove(&(vnode->bros));
		slab_free(&vnodes_slab, vnode);
	}

	return 0;
}

vnode_t *vnodes_find(const vfs_t *fsp, const int index)
{
	const list_t *vnodes = &(fsp->vnodes);
	for (list_t *p = list_next(vnodes); !list_is_edge(vnodes, p);
			p = list_next(p)) {
		vnode_t *vnode = getINodeParent(p);
		if (vnode->index == index)
			return vnode;
	}

	return NULL;
}

void vnodes_synchronize(vfs_t *fsp)
{
	const list_t *vnodes = &(fsp->vnodes);
	for (list_t *p = list_next(vnodes); !list_is_edge(vnodes, p);
			p = list_next(p))
		vfs_synchronize(getINodeParent(p));
}