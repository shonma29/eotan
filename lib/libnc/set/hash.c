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
#include <limits.h>
#include <stddef.h>
#include <set/hash.h>

#define HASH_OK (0)
#define HASH_ERR (1)

#define HASH_MIN_BITS (5)
#define HASH_MAX_BITS (20)

static size_t primes[] = {
	31, 61, 127, 251, 509, 1021, 2039, 4093,
	8191, 16381, 32749, 65521, 131071, 262139, 524287, 1048573
};

extern void *malloc(const size_t size);
extern void free(void *p);

static size_t calc_bits(const size_t max);
static void init_list(list_t *head, hash_entry_t *entries, const size_t size);
static void init_tbl(list_t *p, const size_t size);
static list_t *get_head(const hash_t *h, const void *key);
static list_t *find_entry(const list_t *head, const void *key,
		int (*cmp)(const void *, const void *));


static size_t calc_bits(const size_t max) {
	size_t n;
	unsigned int pat;

	for (n = INT_BIT - 1, pat = 1 << n; n && !(max & pat); n--)
		pat >>= 1;

	return ++n;
}

static void init_list(list_t *head, hash_entry_t *entries, const size_t size) {
	hash_entry_t *p = entries;
	size_t i;

	list_initialize(head);

	for (i = size; i > 0; p++, i--)
		list_append(head, &(p->list));
}

static void init_tbl(list_t *p, const size_t size) {
	size_t i;

	for (i = size; i > 0; i--)
		list_initialize(p++);
}

hash_t *hash_create(const size_t max,
		unsigned int (*calc)(const void *, const size_t size),
		int (*cmp)(const void *, const void *)) {
	hash_t *h;
	size_t bits = calc_bits(max);
	size_t size;

	bits = (bits < HASH_MIN_BITS) ?
			HASH_MIN_BITS : ((bits > HASH_MAX_BITS) ?
					HASH_MAX_BITS : bits);
	size = primes[bits - HASH_MIN_BITS];
	h = (hash_t*)malloc(sizeof(*h) + size * sizeof(list_t));

	if (h) {
		h->entries = (hash_entry_t*)(malloc(
				max * sizeof(hash_entry_t)));

		if (!(h->entries)) {
			free(h);
			h = NULL;
		}
		else {
			init_list(&(h->spares), h->entries, max);
			h->calc = calc;
			h->cmp = cmp;
			h->size = size;
			h->num = 0;
			h->max = max;
			init_tbl(h->tbl, size);
		}
	}

	return h;
}

void hash_destroy(hash_t *h) {
	free(h->entries);
	free(h);
}

void hash_clear(hash_t *h) {
	h->num = 0;
	init_list(&(h->spares), h->entries, h->max);
	init_tbl(h->tbl, h->size);
}

static list_t *get_head(const hash_t *h, const void *key) {
	return (list_t*)&(h->tbl[h->calc(key, h->size)]);
}

static list_t *find_entry(const list_t *head, const void *key,
		int (*cmp)(const void *, const void *)) {
	list_t *p;

	for (p = list_next(head); !list_is_edge(head, p); p = p->next)
		if (!cmp(key, ((hash_entry_t*)p)->key))	return p;

	return NULL;
}

void *hash_get(hash_t *h, const void *key) {
	list_t *head = get_head(h, key);
	list_t *p = find_entry(head, key, h->cmp);

	if (!p)	return NULL;

	if (p != list_next(head)) {
		list_remove(p);
		list_push(head, p);
	}

	return ((hash_entry_t*)p)->value;
}

int hash_put(hash_t *h, const void *key, const void *value) {
	list_t *head = get_head(h, key);
	list_t *p = find_entry(head, key, h->cmp);

	if (!p) {
		p = list_pick(&(h->spares));
		if (!p)	return HASH_ERR;

		h->num++;
		list_push(head, p);
	}

	(((hash_entry_t*)p)->key = (void*)key);
	(((hash_entry_t*)p)->value = (void*)value);

	return HASH_OK;
}

int hash_remove(hash_t *h, const void *key) {
	list_t *p = find_entry(get_head(h, key), key, h->cmp);

	if (!p)	return HASH_ERR;

	h->num--;
	list_remove(p);
	((hash_entry_t*)p)->key = NULL;
	((hash_entry_t*)p)->value = NULL;
	list_push(&(h->spares), p);

	return HASH_OK;
}

size_t hash_size(const hash_t *h) {
	return h->num;
}
