#ifndef _PROCESS_H_
#define _PROCESS_H_
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
#include <mm/segment.h>
#include <mpu/memory.h>
#include <set/list.h>
#include <set/tree.h>

typedef struct {
	node_t node;
	int process_id;
	mm_segment_t stack;
	list_t brothers;
} mm_thread_t;

typedef struct {
	node_t node;
	struct {
		mm_segment_t heap;
		mm_segment_t stack;
	} segments;
	void *directory;
	list_t threads;
} mm_process_t;

extern void process_initialize(void);
extern mm_process_t *get_process(const ID);
extern mm_thread_t *get_thread(const ID);

extern ER default_handler(void);
extern ER stack_fault_handler(void);

#endif
