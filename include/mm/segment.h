#ifndef _MM_SEGMENT_H_
#define _MM_SEGMENT_H_
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

#define SEGMENT_CLASS_MEMORY "memory"
#define SEGMENT_CLASS_SHARED "shared"

typedef enum {
	UNKNOWN = 0,
	MEMORY = 1,
	SHARED = 2
} segment_class_e;

typedef enum {
	attr_nil = 0,
	attr_readable = 1,
	attr_writable = 2,
	attr_executable = 4,
	attr_expandable = 8,
	attr_backward = 16,
	attr_shared = 32,
	attr_detach_on_exec = 64
} mm_segment_attr_e;

typedef enum {
	type_code = attr_readable | attr_executable,
	type_data = attr_readable | attr_writable,
	type_heap = attr_readable | attr_writable | attr_expandable,
	type_stack = attr_readable | attr_writable | attr_expandable
			| attr_backward
} mm_segment_type_e;

typedef struct {
	void *addr;
	size_t len;
	size_t max;
	unsigned int attr;
//	int owner;
	unsigned int ref_count;
	list_t semaphores;
} mm_segment_t;

#endif
