#ifndef _CORE_THREAD_H_
#define _CORE_THREAD_H_
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
#include <core.h>
#include <set/list.h>
#include <set/tree.h>
#include "region.h"
#include "wait.h"
#include "mpu/mpu.h"

typedef struct {
	VP addr;
	SIZE length;
} fragment_t;

typedef struct {
	node_t node;
	list_t queue;
	STAT status;
	wait_reason_t wait;
	T_MPU_CONTEXT mpu;
//TODO is needed?
	VP ustack_top;
//TODO is needed?
	VP kstack_top;
	struct {
		UW total;
		UW left;
	} time;
	ID id;
	PRI priority;
	W (*handler)(W cr2, W eip);
//TODO move to domain
	T_REGION regions[MAX_REGION];
	struct {
		VP_INT arg;
		PRI priority;
		ID domain_id;
		fragment_t ustack;
		fragment_t kstack;
	} attr;
} thread_t;

#endif
