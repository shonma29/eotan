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
#include <kcall.h>
#include <itron/rendezvous.h>
#include "func.h"
#include "mpu/mpufunc.h"

void kcall_initialize(void)
{
	kcall_t *p = (kcall_t*)KCALL_ADDR;

	p->region_create = region_create;
	p->region_destroy = region_destroy;
	p->region_map = region_map;
	p->region_unmap = region_unmap;
	p->region_duplicate = region_duplicate;
	p->region_get = region_get;
	p->region_put = region_put;
	p->region_get_status = region_get_status;

	p->port_create = port_create;
	p->port_create_auto = port_create_auto;
	p->port_destroy = port_destroy;
	p->port_call = port_call;
	p->port_accept = port_accept;
	p->port_reply = port_reply;
	p->queue_create_auto = queue_create_auto;
	p->queue_destroy = queue_destroy;
	p->queue_send = queue_send;
	p->queue_receive = queue_receive;
}
