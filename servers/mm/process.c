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
#include <errno.h>
#include <mm.h>
#include <vm.h>
#include <core/options.h>
#include <mm/segment.h>
#include <nerve/kcall.h>
#include "interface.h"
#include "process.h"

//mm_process_t *processes;
//mm_segment_t *shared;
static kcall_t *kcall = (kcall_t*)KCALL_ADDR;


int mm_process_create(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		//TODO text VM_READ | VM_EXEC | VM_USER
		//TODO data VM_READ | VM_WRITE | VM_USER
		//TODO heap VM_READ | VM_WRITE | VM_USER
		if (kcall->region_create((ID)(args->arg1), (ID)(args->arg2),
				(VP)(args->arg3), (W)(args->arg4),
				(W)(args->arg5),
				VM_READ | VM_WRITE | VM_USER)) {
			reply->error_no = EFAULT;
			break;
		}

		reply->error_no = EOK;
		reply->result = 0;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}

int mm_process_destroy(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		if (kcall->region_destroy((ID)(args->arg1))) {
			reply->error_no = EFAULT;
			break;
		}

		reply->error_no = EOK;
		reply->result = 0;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}

int mm_process_duplicate(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		if (kcall->region_duplicate((ID)(args->arg1), (ID)(args->arg2))) {
			reply->error_no = EFAULT;
			break;
		}

		reply->error_no = EOK;
		reply->result = 0;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}

int mm_process_copy_stack(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		if (kcall->mpu_copy_stack((ID)(args->arg1), (W)(args->arg2),
				(ID)(args->arg3))) {
			reply->error_no = EFAULT;
			break;
		}

		reply->error_no = EOK;
		reply->result = 0;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}

int mm_process_set_context(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		if (kcall->mpu_set_context((ID)(args->arg1), (W)(args->arg2),
				(B*)(args->arg3), (W)(args->arg4))) {
			reply->error_no = EFAULT;
			break;
		}

		reply->error_no = EOK;
		reply->result = 0;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}

int mm_vmap(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		if (kcall->region_map((ID)(args->arg1), (VP)(args->arg2),
				(UW)(args->arg3), (W)(args->arg4))) {
			reply->error_no = ESVC;
			break;
		}

		reply->error_no = EOK;
		reply->result = 0;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}

int mm_vunmap(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		if (kcall->region_unmap((ID)(args->arg1), (VP)(args->arg2),
				(UW)(args->arg3))) {
			reply->error_no = ESVC;
			break;
		}

		reply->error_no = EOK;
		reply->result = 0;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}

int mm_vmstatus(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args)
{
	do {
		if (kcall->region_get_status((ID)(args->arg1), (ID)(args->arg2),
				(VP)(args->arg3))) {
			reply->error_no = EFAULT;
			break;
		}

		reply->error_no = EOK;
		reply->result = 0;
		return reply_success;
	} while (FALSE);

	reply->result = -1;
	return reply_failure;
}
