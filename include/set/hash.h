#ifndef _SET_HASH_H_
#define _SET_HASH_H_
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
#include <set/list.h>

typedef struct {
	list_t list;
	void *key;
	void *value;
} hash_entry_t;

typedef struct {
	unsigned int (*calc)(const void *key, const size_t size);
	int (*cmp)(const void *a, const void *b);
	size_t size;
	size_t num;
	size_t max;
	size_t tupple_size;
	hash_entry_t *entries;
	list_t free;
	list_t tbl[];
} hash_t;

hash_t *hash_create(const size_t max,
		unsigned int (*calc)(const void *key, const size_t size),
		int (*cmp)(const void *a, const void *b));
void hash_destroy(hash_t *h);
void hash_clear(hash_t *h);
void *hash_get(hash_t *h, const void *key);
int hash_put(hash_t *h, const void *key, const void *value);
int hash_remove(hash_t *h, const void *key);
size_t hash_size(const hash_t *h);

#endif
