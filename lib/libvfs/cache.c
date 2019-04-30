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
#include <stdint.h>
#include <string.h>
#include <fs/config.h>
#include <fs/vfs.h>
#include <set/list.h>
#include <set/hash.h>
#include "../libserv/libserv.h"

#define numOf(x) (sizeof(x) / sizeof(x[0]))

typedef struct {
	list_t lru;
	int block_no;
	block_device_t *dev;
	bool dirty;
	unsigned int lock_count;
	//TODO dynamic allocation
	char buf[DEFAULT_BLOCK_SIZE];
} cache_t;

static hash_t *hash;
//TODO dynamic allocation
static cache_t cache[MAX_CACHE];
static cache_t key;
static list_t free_list;
static list_t lru_list;
static list_t locked_list;

static int fill(cache_t *cp);
static int sweep(cache_t *cp);
static void dispose(cache_t *cp);

static unsigned int calc_hash(const void *key, const size_t size);
static int compare(const void *a, const void *b);

static inline cache_t *getLruParent(const list_t *p) {
	return (cache_t*)((intptr_t)p - offsetof(cache_t, lru));
}

static inline cache_t *getBufParent(const void *p) {
	return (cache_t*)((intptr_t)p - offsetof(cache_t, buf));
}

int cache_initialize(void)
{
	hash = hash_create(MAX_CACHE, calc_hash, compare);
	if (!hash)
		return (-1);

	list_initialize(&free_list);
	list_initialize(&lru_list);
	list_initialize(&locked_list);

	for (int i = 0; i < numOf(cache); i++)
		list_append(&free_list, &(cache[i].lru));

	return 0;
}

void *cache_create(block_device_t *dev, const unsigned int block_no)
{
	key.block_no = block_no;
	key.dev = dev;
	cache_t *cp = hash_get(hash, &key);

	if (!cp) {
		if (!list_is_empty(&free_list))
			cp = getLruParent(list_next(&free_list));
		else if (!list_is_empty(&lru_list)) {
			cp = getLruParent(list_next(&lru_list));
			if (cp->dirty)
				if (sweep(cp)) {
					log_debug("cache_create: sweep(%d) error\n",
							block_no);
					return NULL;
				}

			hash_remove(hash, cp);
		} else {
			log_debug("cache_create: no memory\n");
			return NULL;
		}

		cp->dev = dev;
		cp->block_no = block_no;
		cp->dirty = false;
		cp->lock_count = 0;

		hash_put(hash, cp, cp);
	}

	cp->lock_count++;
	list_remove(&(cp->lru));
	list_insert(&locked_list, &(cp->lru));
	memset(cp->buf, 0, sizeof(cp->buf));

//	log_debug("cache_create: %d %d %d %d\n",
//			cp->dev->channel, cp->block_no, cp->lock_count,
//			cp->dirty);
	return cp->buf;
}

void *cache_get(block_device_t *dev, const unsigned int block_no)
{
	key.block_no = block_no;
	key.dev = dev;
	cache_t *cp = hash_get(hash, &key);

	if (!cp) {
		if (!list_is_empty(&free_list))
			cp = getLruParent(list_next(&free_list));
		else if (!list_is_empty(&lru_list)) {
			cp = getLruParent(list_next(&lru_list));
			if (cp->dirty)
				if (sweep(cp)) {
					log_debug("cache_get: sweep(%d) error\n",
							block_no);
					return NULL;
				}

			hash_remove(hash, cp);
		} else {
			log_debug("cache_get: no memory\n");
			return NULL;
		}

		cp->dev = dev;
		cp->block_no = block_no;
		cp->dirty = false;
		cp->lock_count = 0;

		if (fill(cp) != dev->block_size) {
			list_remove(&(cp->lru));
			list_insert(&free_list, &(cp->lru));
			log_debug("cache_get: fill(%d) error\n", block_no);
			return NULL;
		}

		hash_put(hash, cp, cp);
	}

	cp->lock_count++;
	list_remove(&(cp->lru));
	list_insert(&locked_list, &(cp->lru));

//	log_debug("cache_get: %d %d %d %d\n",
//			cp->dev->channel, cp->block_no, cp->lock_count,
//			cp->dirty);
	return cp->buf;
}

bool cache_modify(const void *p)
{
	if (!p)
		return false;

	cache_t *cp = getBufParent(p);
	cp->dirty = true;

//	log_debug("cache_modify: %d %d %d\n",
//			cp->block_no, cp->lock_count, cp->dirty);
	return true;
}

bool cache_release(const void *p, const bool dirty)
{
	if (!p)
		return false;

	cache_t *cp = getBufParent(p);
	cp->dirty |= dirty;

	if (cp->lock_count > 0) {
		cp->lock_count--;
		if (!(cp->lock_count)) {
			list_remove(&(cp->lru));
			list_insert(&lru_list, &(cp->lru));
		}
	}

//	log_debug("cache_release: %d %d %d %d\n",
//			cp->dev->channel, cp->block_no, cp->lock_count,
//			cp->dirty);

	return true;
}

bool cache_invalidate(block_device_t *dev, const unsigned int block_no)
{
	key.block_no = block_no;
	key.dev = dev;
	cache_t *cp = hash_get(hash, &key);

	if (!cp)
		return false;

	dispose(cp);

	return true;
}

int cache_synchronize(block_device_t *dev, const bool unmount)
{
	for (list_t *p = list_next(&lru_list); !list_is_edge(&lru_list, p);) {
		cache_t *cp = getLruParent(p);

//	log_debug("cache_sync: %d %d %d %d\n",
//			cp->dev->channel, cp->block_no, cp->lock_count,
//			cp->dirty);
		if (cp->dev->channel != dev->channel) {
			p = list_next(p);
			continue;
		}

		if (cp->dirty) {
			int error_no = sweep(cp);
			if (error_no)
				return error_no;
			cp->dirty = false;
		}

		p = list_next(p);

		if (unmount)
			dispose(cp);
	}

	return 0;
}

static int fill(cache_t *cp)
{
	return cp->dev->read(cp->dev, cp->buf, cp->block_no);
}

static int sweep(cache_t *cp)
{
	return cp->dev->write(cp->dev, cp->buf, cp->block_no);
}

static void dispose(cache_t *cp)
{
	list_remove(&(cp->lru));
	list_insert(&free_list, &(cp->lru));
	hash_remove(hash, cp);
}

static unsigned int calc_hash(const void *key, const size_t size)
{
	cache_t *p = (cache_t*)key;

	return (p->block_no % size);
}

static int compare(const void *a, const void *b)
{
	cache_t *x = (cache_t *)a;
	cache_t *y = (cache_t *)b;

	if (x->block_no == y->block_no) {
			if (x->dev->channel == y->dev->channel)
				return 0;
			else if (x->dev->channel < y->dev->channel)
				return (-1);
			else
				return 1;
	} else if (x->block_no < y->block_no)
		return (-1);
	else
		return 1;
}
