#ifndef _SET_LIST_H_
#define _SET_LIST_H_
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
#include "stdbool.h"
#include "stddef.h"

typedef struct _list_t {
	struct _list_t *next;
	struct _list_t *prev;
} list_t;


static inline list_t *list_next(const list_t *entry)
{
	return entry->next;
}

static inline list_t *list_prev(const list_t *entry)
{
	return entry->prev;
}

static inline bool list_is_empty(const list_t *entry)
{
	return (entry->next == entry);
}

static inline bool list_is_edge(const list_t *guard, const list_t *entry)
{
	return (guard == entry);
}

static inline list_t *list_edge_to_null(const list_t *guard,
		const list_t *entry)
{
	return ((guard == entry) ? NULL : ((list_t *) entry));
}

static inline list_t *list_head(const list_t *guard)
{
	return list_edge_to_null(guard, guard->next);
}

static inline list_t *list_tail(const list_t *guard)
{
	return list_edge_to_null(guard, guard->prev);
}

extern void list_initialize(list_t *);
extern void list_release(list_t *);

extern void list_append(list_t *, list_t *);
extern void list_insert(list_t *, list_t *);
extern void list_remove(list_t *);

extern list_t *list_pick(list_t *);

static inline void list_push(list_t *guard, list_t *entry)
{
	list_insert(guard, entry);
}

static inline list_t *list_pop(list_t *guard)
{
	return list_pick(guard);
}

static inline void list_enqueue(list_t *guard, list_t *entry)
{
	list_append(guard, entry);
}

static inline list_t *list_dequeue(list_t *guard)
{
	return list_pick(guard);
}

#endif
