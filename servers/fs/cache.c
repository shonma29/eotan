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
#include <stdbool.h>
#include <set/list.h>
#include "fs.h"

#define HASH_SIZE (31)
#define CACHE_SIZE (200)

#define numOf(x) (sizeof(x) / sizeof(x[0]))

typedef struct {
	list_t lru;
	list_t hash;
	W blockno;
	W device_id;
	bool dirty;
	B buf[CACHE_BLOCK_SIZE];
} cache_t;

static cache_t cache[CACHE_SIZE];
static list_t free_list;
static list_t lru_list;
static list_t hash_list[HASH_SIZE];

static W fill(cache_t *cp);
static W sweep(cache_t *cp);
static void dispose(cache_t *cp);
static list_t *lookup(const W blockno);
static cache_t *find(list_t *h, const W device_id, const W blockno);

static inline cache_t *getLruParent(const list_t *p) {
	return (cache_t*)((intptr_t)p - offsetof(cache_t, lru));
}

static inline cache_t *getHashParent(const list_t *p) {
	return (cache_t*)((intptr_t)p - offsetof(cache_t, hash));
}

static inline cache_t *getBufParent(const void *p) {
	return (cache_t*)((intptr_t)p - offsetof(cache_t, buf));
}

void init_cache(void)
{
	list_initialize(&free_list);
	list_initialize(&lru_list);

	for (int i = 0; i < numOf(hash_list); i++)
		list_initialize(&(hash_list[i]));

	for (int i = 0; i < numOf(cache); i++) {
		cache_t *cp = &(cache[i]);

		list_append(&free_list, &(cp->lru));
		list_initialize(&(cp->hash));
	}
}

void *get_cache(const W device_id, const W blockno)
{
	list_t *h = lookup(blockno);
	cache_t *cp = find(h, device_id, blockno);

	if (!cp) {
		if (!list_is_empty(&free_list))
			cp = getLruParent(list_next(&free_list));
		else if (!list_is_empty(&lru_list)) {
			cp = getLruParent(list_next(&lru_list));
			if (cp->dirty)
				if (sweep(cp))
					return NULL;

			list_remove(&(cp->hash));
		} else
			return NULL;

		cp->dirty = false;
		cp->device_id = device_id;
		cp->blockno = blockno;

		if (fill(cp)) {
			list_remove(&(cp->lru));
			list_insert(&free_list, &(cp->lru));
			return NULL;
		}

		list_insert(h, &(cp->hash));
	}

	list_remove(&(cp->lru));
	list_insert(&lru_list, &(cp->lru));

	return cp->buf;
}

bool invalidate_cache(const W device_id, const W blockno)
{
	cache_t *cp = find(lookup(blockno), device_id, blockno);

	if (!cp)
		return false;

	dispose(cp);

	return true;
}

bool put_cache(const void *p, const bool dirty)
{
	if (!p)
		return false;

	cache_t *cp = getBufParent(p);

	if (dirty)
		cp->dirty = dirty;

	list_remove(&(cp->lru));
	list_insert(&lru_list, &(cp->lru));

	return true;
}

W sync_cache(const W device_id, const bool unmount)
{
	for (list_t *p = list_next(&lru_list); !list_is_edge(&lru_list, p);
			p = p->next) {
		cache_t *cp = getLruParent(p);

		if (cp->device_id != device_id)
			continue;

		if (cp->dirty) {
			W error_no = sweep(cp);
			if (error_no)
				return error_no;
		}

		if (unmount)
			dispose(cp);
	}

	return EOK;
}

static W fill(cache_t *cp)
{
	W rsize;
	W error_no = read_device(cp->device_id, cp->buf,
			cp->blockno * sizeof(cache[0].buf),
			sizeof(cache[0].buf), &rsize);

	if (error_no)
		dbg_printf("fs: fill failed(%d)\n", error_no);

	return error_no;
}

static W sweep(cache_t *cp)
{
	W rsize;
	W error_no = write_device(cp->device_id, cp->buf,
			cp->blockno * sizeof(cache[0].buf),
			sizeof(cache[0].buf), &rsize);

	if (error_no)
		dbg_printf("fs: sweep failed(%d)\n", error_no);

	return error_no;
}

static void dispose(cache_t *cp)
{
	list_remove(&(cp->lru));
	list_insert(&free_list, &(cp->lru));
	list_remove(&(cp->hash));
}

static list_t *lookup(const W blockno)
{
	return &(hash_list[blockno % numOf(hash_list)]);
}

static cache_t *find(list_t *h, const W device_id, const W blockno)
{
	for (list_t *p = list_next(h); !list_is_edge(h, p); p = p->next) {
		cache_t *cp = getHashParent(p);

		if ((cp->blockno == blockno)
				&& (cp->device_id == device_id))
			return cp;
	}

	return NULL;
}
