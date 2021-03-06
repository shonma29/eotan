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
#include <set/slab.h>


static slab_block_t *slab_add_block(slab_t *slab)
{
	if (slab->block_num >= slab->max_block)
		return NULL;

	slab_block_t *block = (slab_block_t *) slab->palloc();
	if (block) {
		list_enqueue(&(slab->empties), &(block->empties));
		slab->empty_num++;
		list_enqueue(&(slab->blocks), &(block->blocks));
		slab->block_num++;
		list_initialize(&(block->entries));
		block->entry_num = slab->entries_per_block;

		slab_entry_t *entry = (slab_entry_t *) ((uintptr_t) block
				+ sizeof(slab_block_t)
				+ slab->offset);
		for (unsigned int i = 0; i < slab->entries_per_block; i++) {
			list_enqueue(&(block->entries), &(entry->entries));
			entry = (slab_entry_t *) ((uintptr_t) entry
					+ slab->unit_size);
		}
	}

	return block;
}

int slab_create(slab_t *slab)
{
	if (!slab->block_size
			|| slab->unit_size < sizeof(slab_entry_t)
			|| (slab->unit_size + sizeof(slab_block_t)
					> slab->block_size)
			|| !slab->max_block
			|| !slab->palloc
			|| !slab->pfree)
		return SLAB_ERROR;

	slab->entries_per_block = (slab->block_size - sizeof(slab_block_t))
			/ slab->unit_size;
	slab->mask = ~(slab->block_size - 1);
	slab->offset = 0;
	list_initialize(&(slab->empties));
	slab->empty_num = 0;
	list_initialize(&(slab->blocks));
	slab->block_num = 0;

	for (unsigned int i = 0; i < slab->min_block; i++)
		if (!slab_add_block(slab))
			break;

	return SLAB_OK;
}

void *slab_alloc(slab_t *slab)
{
	slab_block_t *block = (slab_block_t *) list_head(&(slab->empties));
	if (!block) {
		if (slab_add_block(slab))
			block = (slab_block_t *) list_head(&(slab->empties));
		else
			return NULL;
	}

	block->entry_num--;
	if (!block->entry_num) {
		list_remove(&(block->empties));
		slab->empty_num--;
	}

	return list_pop(&(block->entries));
}

void slab_free(slab_t *slab, void *p)
{
	slab_block_t *block = (slab_block_t *) (((uintptr_t) p) & slab->mask);

	if (!block->entry_num) {
		list_enqueue(&(slab->empties), &(block->empties));
		slab->empty_num++;
	}

	block->entry_num++;
	if ((block->entry_num == slab->entries_per_block)
			&& (slab->empty_num > 1)
			&& (slab->block_num > slab->min_block)) {
		list_remove(&(block->empties));
		slab->empty_num--;
		list_remove(&(block->blocks));
		slab->block_num--;
		slab->pfree(block);
	} else
		list_push(&(block->entries),
				&(((slab_entry_t *) p)->entries));
}
