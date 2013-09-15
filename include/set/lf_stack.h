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
#include "stddef.h"

typedef struct _lf_stack_entry_t {
	struct _lf_stack_entry_t *next;
	char buf[0];
} lf_stack_entry_t;

typedef struct _lf_stack_head_t {
	lf_stack_entry_t *next;
	size_t count;
} lf_stack_head_t;

typedef struct _lf_stack_t {
	lf_stack_head_t head;
	size_t entry_size;
} lf_stack_t;


static inline lf_stack_entry_t *stack_next(lf_stack_entry_t *entry)
{
	return entry->next;
}

static inline void *stack_buf(lf_stack_entry_t *entry)
{
	return (void*)(entry->buf);
}

static inline int stack_is_empty(lf_stack_t *stack)
{
	return !(stack->head.next);
}

extern size_t stack_entry_size(size_t size);
extern size_t stack_buf_size(size_t size, size_t entry_num);
extern void stack_initialize(lf_stack_t *stack, void *buf,
	size_t size, size_t entry_num);

extern void stack_push(volatile lf_stack_t *stack, void *p);
extern void *stack_pop(volatile lf_stack_t *stack);

#endif
