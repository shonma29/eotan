#ifndef LIST_H
#define LIST_H
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
#include "stddef.h"

typedef struct _list_t {
	struct _list_t *next;
	struct _list_t *prev;
} list_t;


#define list_next(entry) ((entry)->next)
#define list_prev(entry) ((entry)->prev)

#define list_is_empty(entry) (((entry)->next) == entry)

#define list_is_edge(guard, entry) ((guard) == (entry))
#define list_edge_to_null(guard, entry) (list_is_edge(guard, entry)? \
  NULL:(entry))

#define list_push(guard, entry) list_append(guard, entry)
#define list_pop(guard) list_pick(guard)

#define list_enqueue(guard, entry) list_insert(guard, entry)
#define list_dequeue(guard) list_pick(guard)


extern void list_initialize(list_t *entry);
extern void list_release(list_t *guard);

extern list_t *list_head(const list_t *guard);
extern list_t *list_tail(const list_t *guard);

extern void list_append(list_t *to, list_t *entry);
extern void list_insert(list_t *to, list_t *entry);
extern void list_remove(list_t *entry);

extern list_t *list_pick(list_t *guard);

#endif
