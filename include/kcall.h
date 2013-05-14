#ifndef _KCALL_H_
#define _KCALL_H_
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
#include <itron/dataqueue.h>
#include <itron/rendezvous.h>

#define KCALL_ADDR 0x80003f00

typedef struct {
	ER (*region_create)(ID id, ID rid, VP start, W min, W max, UW perm);
	ER (*region_destroy)(ID id, ID rid);
	ER (*region_map)(ID id, VP start, UW size, W accmode);
	ER (*region_unmap)(ID id, VP start, UW size);
	ER (*region_duplicate)(ID src, ID dst);
	ER (*region_get)(ID id, VP start, UW size, VP buf);
	ER (*region_put)(ID id, VP start, UW size, VP buf);
	ER (*region_get_status)(ID id, ID rid, VP stat);
	ER (*port_create)(ID porid, T_CPOR *pk_cpor);
	ER_ID (*port_create_auto)(T_CPOR *pk_cpor);
	ER (*port_destroy)(ID porid);
	ER_UINT (*port_call)(ID porid, VP msg, UINT cmsgsz);
	ER_UINT (*port_accept)(ID porid, RDVNO *p_rdvno, VP msg);
	ER (*port_reply)(RDVNO rdvno, VP msg, UINT rmsgsz);
	ER_ID (*queue_create_auto)(T_CDTQ *pk_cdtq);
	ER (*queue_destroy)(ID dtqid);
	ER (*queue_send)(ID dtqid, VP_INT data);
	ER (*queue_receive)(ID dtqid, VP_INT *p_data);
} kcall_t;

#endif