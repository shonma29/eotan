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
#include <icall.h>
#include <global.h>
#include "delay.h"
#include "func.h"

static ER delayed_thread_start(ID thread_id);
static ER delayed_queue_send_nowait(ID queue_id, VP_INT data);
static ER enqueue(delay_param_t *param);


void icall_initialize(void)
{
	icall_t *p = (icall_t*)ICALL_ADDR;

	p->thread_start = delayed_thread_start;
	p->thread_get_id = thread_get_id;
	p->queue_send_nowait = delayed_queue_send_nowait;
	p->puts = putsk;
}

static ER delayed_thread_start(ID thread_id)
{
	delay_param_t param;

	param.action = delay_start;
	param.arg1 = (int)thread_id;

	return enqueue(&param);
}

static ER delayed_queue_send_nowait(ID queue_id, VP_INT data)
{
	delay_param_t param;

	param.action = delay_send;
	param.arg1 = (int)queue_id;
	param.arg2 = (int)data;

	return enqueue(&param);
}

static ER enqueue(delay_param_t *param)
{
	system_info_t *info = (system_info_t*)SYSTEM_INFO_ADDR;

	if (lfq_enqueue(&(info->kqueue), param) != QUEUE_OK)
		return E_TMOUT;

	delay_start = TRUE;

	return E_OK;
}
