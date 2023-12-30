#ifndef SET_LF_STACK_H
#define SET_LF_STACK_H
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
#include <stdint.h>

typedef struct _lfs_entry_t {
	struct _lfs_entry_t *next;
	char buf[0];
} lfs_entry_t;

typedef struct _lfs_head_t {
	lfs_entry_t *next;
	unsigned long count;
} lfs_head_t;

typedef struct _lfs_t {
	lfs_head_t head;
	size_t entry_size;
} lfs_t;

static inline lfs_entry_t *lfs_next(lfs_entry_t *entry)
{
	return entry->next;
}

static inline void *lfs_buf(lfs_entry_t *entry)
{
	return ((void *) (entry->buf));
}

static inline int lfs_is_empty(lfs_t *stack)
{
	return !(stack->head.next);
}

#define lfs_entry_size(size) \
	((size + sizeof(lfs_entry_t) + sizeof(uintptr_t) - 1) \
			& ~(sizeof(uintptr_t) - 1))

#define lfs_buf_size(size, entry_num) \
	(lfs_entry_size(size) * entry_num)

extern void lfs_initialize(volatile lfs_t *, void *, size_t, size_t);
extern void lfs_push(volatile lfs_t *, void *);
extern void *lfs_pop(volatile lfs_t *);

#endif
