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
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpu/memory.h>

#define INITIAL_SIZE (PAGE_SIZE * 8)
#define MASK_USING (1)

typedef struct _Fragment {
	struct _Fragment *next;
	struct _Fragment *prev;
	char buf[0];
} Fragment;

#define MIN_SIZE sizeof(Fragment)

static Fragment free_fragments;
static Fragment *heap_start = NULL;
static Fragment *heap_end = NULL;

static void list_insert(Fragment *, Fragment *);
static void list_remove(Fragment *);
static void block_append(Fragment *, Fragment *);
static void *find(const size_t);
static bool expand(const size_t);


static inline void set_using(Fragment *entry)
{
	uintptr_t *p = (uintptr_t *) &(entry->next);
	*p |= MASK_USING;
}

static inline void unset_using(Fragment *entry)
{
	uintptr_t *p = (uintptr_t *) &(entry->next);
	*p &= ~MASK_USING;
}

static inline bool is_using(const Fragment *entry)
{
	uintptr_t *p = ((uintptr_t *) &(entry->next));
	return (*p & MASK_USING);
}

static inline void list_initialize(Fragment *entry)
{
	entry->prev = entry->next = entry;
}

static void list_insert(Fragment *to, Fragment *entry)
{
	Fragment *next = to->next;

	entry->next = next;
	entry->prev = to;
	next->prev = to->next = entry;
}

static void list_remove(Fragment *entry)
{
	Fragment *next = entry->next;
	Fragment *prev = entry->prev;

	next->prev = prev;
	prev->next = next;
}

static void block_append(Fragment *to, Fragment *entry)
{
	Fragment *prev = to->prev;
	entry->next = (Fragment *) (((uintptr_t) to) | MASK_USING);
	entry->prev = prev;
	to->prev = entry;
	prev->next = (Fragment *) (((uintptr_t) entry)
			| (((uintptr_t) prev->next) & MASK_USING));

	free(&(entry[1]));
}

void __malloc_initialize(void)
{
	list_initialize(&free_fragments);

	heap_start = (Fragment *) sbrk(INITIAL_SIZE);
	if (((uintptr_t) heap_start) == -1)
		return;

	heap_end = (Fragment *) (((uintptr_t) heap_start) + INITIAL_SIZE);
	list_initialize(heap_start);
	list_insert(heap_start, &(heap_start[1]));
	set_using(heap_start);
	list_insert(&free_fragments, &(heap_start[2]));
}

void *malloc(size_t size)
{
	if (!size)
		return NULL;

	size = (size < MIN_SIZE) ? MIN_SIZE : roundUp(size, MIN_SIZE);

	void *p;
	for (; !(p = find(size));)
		if (!heap_end || !expand(size)) {
			_set_local_errno(ENOMEM);
			break;
		}
	return p;
}

static void *find(const size_t size)
{
	for (Fragment *p = free_fragments.next; p != &free_fragments;
			p = p->next) {
		Fragment *f = &(p[-1]);
		size_t space = (uintptr_t) ((f->next == heap_start) ?
						heap_end : (f->next))
				- (uintptr_t) (f->buf);

		if (space < size)
			continue;

		list_remove(p);

		if (space - size >= MIN_SIZE * 2) {
			Fragment *next = (Fragment *) &(((char *) p)[size]);
			list_insert(f, next);
			list_insert(&free_fragments, &(next[1]));
		}

		set_using(f);
		return p;
	}

	return NULL;
}

static bool expand(const size_t size)
{
	size_t need = pageRoundUp(sizeof(Fragment) + size);
	if (((uintptr_t) sbrk(need)) == -1)
		return false;

	Fragment *entry = heap_end;
	heap_end = (Fragment *) (((uintptr_t) heap_end) + need);
	block_append(heap_start, entry);
	return true;
}

void free(void *p)
{
	if ((((uintptr_t) p) & (MIN_SIZE - 1))
			|| ((uintptr_t) p < (uintptr_t) &(heap_start[2]))
			|| ((uintptr_t) heap_end <= (uintptr_t) p))
		return;

	Fragment *f = &(((Fragment *) p)[-1]);
	if (!is_using(f))
		return;

	unset_using(f);

	if (!is_using(f->next)) {
		Fragment *next = f->next;
		next->prev->next = f;
		next->next->prev = f;
		f->next = next->next;
		list_remove(&next[1]);
	}

	if (!is_using(f->prev)) {
		Fragment *prev = f->prev;
		prev->next = f->next;
		f->next->prev = prev;
		return;
	}

	list_insert(&free_fragments, p);
}
