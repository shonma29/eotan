#ifndef _NERVE_KCALL_H_
#define _NERVE_KCALL_H_
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
#include <nerve/config.h>

typedef struct {
	void (*dispatch)(void);
	ER_ID (*thread_create_auto)(T_CTSK *pk_ctsk);
	ER (*thread_destroy)(ID tskid);
	ER (*thread_start)(ID tskid);
	void (*thread_end_and_destroy)(void);
	ER (*thread_terminate)(ID tskid);
	ER (*time_get)(SYSTIM *p_systim);
	ER (*time_set)(SYSTIM *p_systim);
	ER (*interrupt_bind)(const INHNO inhno, const T_DINH *pk_dinh);
	ER (*interrupt_enable)(const UB ir);
	void *(*palloc)(void);
	void (*pfree)(void *addr);
	ER (*region_get)(const ID id, const void *from, const size_t size,
			void *to);
	ER (*region_put)(const ID id, void *to, const size_t size,
			const void *from);
	ER_UINT (*region_copy)(const ID id, const void *from, const size_t size,
			void *to);
	ER (*port_create)(ID porid, T_CPOR *pk_cpor);
	ER_ID (*port_create_auto)(T_CPOR *pk_cpor);
	ER (*port_destroy)(ID porid);
	ER_UINT (*port_call)(ID porid, VP msg, UINT cmsgsz);
	ER_UINT (*port_accept)(ID porid, RDVNO *p_rdvno, VP msg);
	ER (*port_reply)(RDVNO rdvno, VP msg, UINT rmsgsz);
	ER_ID (*queue_create_auto)(T_CDTQ *pk_cdtq);
	ER (*queue_destroy)(ID dtqid);
	ER (*queue_send)(ID dtqid, VP_INT data, TMO tmout);
	ER (*queue_receive)(ID dtqid, VP_INT *p_data);
	ER_ID (*mutex_create_auto)(T_CMTX *pk_cmtx);
	ER (*mutex_destroy)(ID mtxid);
	ER (*mutex_lock)(ID mtxid, TMO tmout);
	ER (*mutex_unlock)(ID mtxid);
	void (*puts)(const char *str);
} kcall_t;

#endif
