#ifndef SET_SLAB_H
#define SET_SLAB_H
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
#include "set/list.h"

#define SLAB_OK 0
#define SLAB_ERROR (-1)

typedef struct _slab_t {
	size_t unit_size;
	size_t block_size;
	size_t min_block;
	size_t max_block;
	void *(*palloc)(void);
	void (*pfree)(void *p);
	size_t entries_per_block;
	ptr_t mask;
	size_t offset;
	list_t empties;
	size_t empty_num;
	list_t blocks;
	size_t block_num;
} slab_t;

typedef struct _slab_block_t {
	list_t empties;
	list_t blocks;
	list_t entries;
	size_t entry_num;
} slab_block_t;

typedef struct _slab_entry_t {
	list_t entries;
} slab_entry_t;


extern int slab_create(slab_t *slab);
extern void slab_destroy(slab_t *slab);
extern void *slab_alloc(slab_t *slab);
extern void slab_free(slab_t *slab, void *p);

#endif
