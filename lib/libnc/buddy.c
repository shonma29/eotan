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
#if 0
#include <stdio.h>
#include <stdlib.h>

#define FLG_FREE 0

#define IS_FREE(q) (!(q->using.isFree))
#define SET_FREE(q) (q->using.isFree = FLG_FREE)

typedef struct _list {
	struct _list *next;
	struct _list *prev;
} list;

typedef struct _pair {
	unsigned int isFree;
	unsigned int multi;
} pair;

typedef union _Buddy {
	list free;
	pair using;
} Buddy;

#define PAGE_SIZE 4096
#define MIN_BITS 3
#define MAX_BITS 12
#define SIZ_MULTI (MAX_BITS - MIN_BITS + 1)


/* { 2^3, 2^4, 2^5, 2^6, 2^7, 2^8, 2^9, 2^10, 2^11, 2^12 } */
static Buddy buddies[SIZ_MULTI];
static void *(*pageAllocator)(size_t size);
static void (*pageDeallocator)(void *p);

void initialize(void *(*alloc)(size_t size), void (*dealloc)(void *p)) {
	int i;

	for (i = 0; i < SIZ_MULTI; i++) {
		Buddy *p = &(buddies[i]);
		p->free.next = &(p->free);
		p->free.prev = &(p->free);
	}

	pageAllocator = alloc;
	pageDeallocator = dealloc;
}

static unsigned int getMulti(size_t size) {
	return (size + sizeof(Buddy) + ((2 << MIN_BITS) - 1)) >> MIN_BITS;
}

//TODO replace with list library
static void push(list *dest, list *src) {
	src->next = dest->next;
	src->prev = dest;
	dest->next->prev = src;
	dest->next = src;
}

static void cut(list *p) {
	p->next->prev = p->prev;
	p->prev->next = p->next;
}

static list *pop(list *p) {
	cut(p->next);

	return p->next;
}

void *allocate(const size_t size) {
	unsigned int multi = getMulti(size);
	Buddy *p = NULL;
	int i;

	if (multi >= SIZ_MULTI) {
		size_t realSize = (size + sizeof(Buddy) + (PAGE_SIZE -1))
				>> MAX_BITS;

		p = pageAllocator(realSize << MAX_BITS);

		if (p)	p->using.multi = multi;

		return &p[1];
	}

	for (i = multi; i < SIZ_MULTI; i++) {
		if (buddies[i].free.next == &(buddies[i].free)) {
			p = (Buddy*)pop(&(buddies[i].free));
			break;
		}
	}

	if (!p) {
		p = pageAllocator(PAGE_SIZE);
		i--;
	}

	if (p) {
		for (; i > multi; i--) {
			//TODO split
			p = (Buddy*)(&(buddies[multi].free.next));
		}
	}

	return p;
}

void release(void *b) {
	Buddy *p = (Buddy*)((unsigned int)b - sizeof(Buddy));
	unsigned int multi = p->using.multi;

	if (multi >= SIZ_MULTI) {
		pageDeallocator(p);
		return;
	}

	for (; multi < SIZ_MULTI; multi++) {
		int right = ((unsigned int)p) & (2 << (MIN_BITS + multi));
		//TODO add body size
		Buddy *pair = right ? (p - 1) : (p + 1);

		if (IS_FREE(pair)) {
			cut(&(pair->free));
			p = right ? pair : p;
		}
		else {
			SET_FREE(p);
			push(&(buddies[multi].free), &(p->free));
			break;
		}
	}
}

int main(int argc, char **argv) {
	Buddy x;
	Buddy *p = &x;

	p->free.prev = (list*)0x8;
	p->free.next = (list*)0x16;

	printf("next = %p, prev = %p\n", p->free.next, p->free.prev);
	printf("isfree = %p\n", IS_FREE(p));
	SET_FREE(p);
	printf("isfree = %p\n", IS_FREE(p));

	initialize(malloc, free);

	return 0;
}
#endif
