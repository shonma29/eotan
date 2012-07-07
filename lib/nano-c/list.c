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
#include "set/list.h"


void list_initialize(list *entry) {
	entry->prev = entry->next = entry;
}

void list_release(list *guard) {
	list *entry = guard;

	do {
		list *next = entry->next;

		list_initialize(entry);
		entry = next;
	} while (!list_is_edge(guard, entry));
}

list *list_head(const list *guard) {
	return list_edge_to_null(guard, guard->next);
}

list *list_tail(const list *guard) {
	return list_edge_to_null(guard, guard->prev);
}

void list_append(list *to, list *entry) {
	list *next = to->next;

	entry->next = next;
	entry->prev = to;
	next->prev = to->next = entry;
}

void list_insert(list *to, list *entry) {
	list *prev = to->prev;

	entry->next = to;
	entry->prev = prev;
	prev->next = to->prev = entry;
}

void list_remove(list *entry) {
	list *next = entry->next;
	list *prev = entry->prev;

	next->prev = prev;
	prev->next = next;
	list_initialize(entry);
}

list *list_pick(list *guard) {
	list *entry = guard->next;

	if (list_is_edge(guard, entry))	entry = NULL;
	else	list_remove(entry);

	return entry;
}
