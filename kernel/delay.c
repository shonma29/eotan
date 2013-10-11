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
#include <kthread.h>
#include <mpu/config.h>
#include <mpu/io.h>
#include <set/lf_queue.h>
#include "func.h"
#include "setting.h"
#include "mpu/mpufunc.h"

#define KQUEUE_SIZE 1024

static char kqbuf[lfq_buf_size(sizeof(ID), KQUEUE_SIZE)];
volatile lfq_t kqueue;
ID delay_thread_id = 0;

static ER_ID attach(void);
static void process(void);
static void detach(void);

kthread_t delay_thread = { attach, process, detach };


static ER_ID attach(void)
{
	T_CTSK pk_ctsk = {
		TA_HLNG, 0, process, pri_dispatcher,
		KERNEL_STACK_SIZE, NULL, KERNEL_DOMAIN_ID
	};

	lfq_initialize(&kqueue, kqbuf, sizeof(ID), KQUEUE_SIZE);

	return thread_create_auto(&pk_ctsk);
}

static void process(void)
{
	for (;;) {
		ID id;

		if (do_timer) {
			check_timer();
			do_timer = FALSE;
		}

		if (lfq_dequeue(&kqueue, &id) != QUEUE_OK)
			break;

		posix_kill_proc(id);
	}
}

static void detach(void)
{
}

void kern_start(void)
{
	if (core_initialize())
		panic("failed to initialize");

	for (;;)
		halt();
}
