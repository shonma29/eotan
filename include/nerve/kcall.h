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
	ID (*thread_get_id)(void);
	ER_ID (*thread_create_auto)(T_CTSK *);
	ER (*thread_destroy)(ID);
	ER (*thread_start)(ID);
	void (*thread_end_and_destroy)(void);
	ER (*thread_terminate)(ID);
	void (*thread_tick)(void);
	ER (*thread_sleep)(void);
	void *(*palloc)(void);
	void (*pfree)(void *);
	ER (*region_get)(const ID, const void *, const size_t, void *);
	ER (*region_put)(const ID, void *, const size_t, const void *);
	ER_UINT (*region_copy)(const ID, const void *, const size_t , void *);
	ER (*port_create)(ID porid, T_CPOR *);
	ER_ID (*port_create_auto)(T_CPOR *);
	ER (*port_destroy)(ID);
	ER_UINT (*port_call)(ID, VP, UINT);
	ER_UINT (*port_accept)(ID, RDVNO *, VP);
	ER (*port_reply)(RDVNO, VP, UINT);
	ER_ID (*queue_create_auto)(T_CDTQ *);
	ER (*queue_destroy)(ID);
	ER (*queue_send)(ID, VP_INT, TMO);
	ER (*queue_receive)(ID, VP_INT *);
	ER_ID (*mutex_create_auto)(T_CMTX *);
	ER (*mutex_destroy)(ID);
	ER (*mutex_lock)(ID, TMO);
	ER (*mutex_unlock)(ID);
	void (*printk)(const char *, ...);
} kcall_t;

#define kcall ((kcall_t*)KCALL_ADDR)

#endif
