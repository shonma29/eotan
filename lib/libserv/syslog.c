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
#include <device.h>
#include <kcall.h>
#include <services.h>
#include <string.h>
#include <itron/rendezvous.h>
#include "../../servers/kernlog/kernlog.h"


ER syslog(B *msg)
{
	devmsg_t packet;
	size_t len = strlen(msg);
	kcall_t *kcall = (kcall_t*)KCALL_ADDR;

	packet.req.header.msgtyp = DEV_WRI;
	packet.req.body.wri_req.dd = DESC_SYSLOG;
	packet.req.body.wri_req.start = 0;
	packet.req.body.wri_req.size = len;
	memcpy(packet.req.body.wri_req.dt, msg, len);

	return kcall->port_call(PORT_SYSLOG, &packet,
			sizeof(packet.req.header)
			+ sizeof(packet.req.body.wri_req)
			- sizeof(packet.req.body.wri_req.dt)
			+ len);
}
