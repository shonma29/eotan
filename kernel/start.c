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
#include <services.h>
#include <stdalign.h>
#include <stdnoreturn.h>
#include <nerve/global.h>
#include "func.h"
#include "ready.h"
#include "mpu/mpufunc.h"

static ER create_idle_thread(const VP_INT);
static noreturn void idle_start(VP_INT);


void start(void (*callback)(void))
{
	sysinfo->sync.state.interrupt_nest = 0;
	sysinfo->sync.state.no_request = 1;
	sysinfo->sync.state.serializing = 0;
	sysinfo->delay_thread_start = false;

	context_initialize();
	kcall_initialize();
	service_initialize();
	ipc_initialize();
	mutex_initialize();
	thread_initialize();
	create_idle_thread((VP_INT) callback);
}

//TODO move to starter
static ER create_idle_thread(const VP_INT exinf)
{
	static T_CTSK pk_ctsk = {
		TA_HLNG | TA_ACT,
		(VP_INT) NULL,
		(FP) idle_start,
		MAX_PRIORITY,
		KTHREAD_STACK_SIZE,
		NULL,
		NULL,
		NULL
	};
	pk_ctsk.exinf = exinf;

	thread_t dummy;
	dummy.status = TTS_DMT;
	running = &dummy;

	return thread_create(PORT_IDLE, &pk_ctsk);
}

static noreturn void idle_start(VP_INT exinf)
{
	ei();

	void (*callback)(void) = (void (*)(void)) exinf;
	callback();

	//TODO ugly. boot codes were released while di
	ei();
	for (;;)
		halt();
}
