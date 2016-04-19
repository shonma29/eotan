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
#include <device.h>
#include <services.h>
#include <stddef.h>
#include <nerve/kcall.h>
#include <sys/types.h>
#include "../../lib/libserv/libserv.h"
#include "pts.h"


int detach(void)
{
	return 0;
}

int read(unsigned char *outbuf, const int channel,
		const off_t start, const size_t size)
{
	off_t rpos = start;
	off_t wpos = 0;
	size_t rest = size;

	while (rest > 0) {
		ER_UINT result;
		size_t len = (rest < DEV_BUF_SIZE)? rest:DEV_BUF_SIZE;
		devmsg_t packet;
		devmsg_t *p = &packet;

		packet.Rread.operation = operation_read;
		packet.Rread.channel = channel;
		packet.Rread.offset = rpos;
		packet.Rread.length = len;
		packet.Rread.data = &(outbuf[wpos]);

		result = kcall->port_call(PORT_KEYBOARD, &p, sizeof(p));
		if (result != 0) {
			dbg_printf("pts: cal_por failed(%d)\n", result);
			return -1;
		}

		else if (packet.Tread.length != len) {
			dbg_printf("pts: read icompletely\n");
			return -1;
		}

		rpos += len;
		wpos += len;
		rest -= len;
	}

	return size;
}

int write(unsigned char *inbuf, const int channel,
		const off_t start, const size_t size) {
	off_t rpos = 0;
	off_t wpos = start;
	size_t rest = size;

	while (rest > 0) {
		ER_UINT result;
		size_t len = (rest < DEV_BUF_SIZE)? rest:DEV_BUF_SIZE;
		devmsg_t packet;
		devmsg_t *p = &packet;

		packet.Rwrite.operation = operation_write;
		packet.Rwrite.channel = channel;
		packet.Rwrite.offset = wpos;
		packet.Rwrite.length = len;
		packet.Rwrite.data = &(inbuf[rpos]);

		result = kcall->port_call(PORT_CONSOLE, &p, sizeof(p));
		if (result != 0) {
			dbg_printf("pts: cal_por failed(%d)\n", result);
			return -1;
		}

		else if (packet.Twrite.length != len) {
			dbg_printf("pts: wrote icompletely\n");
			return -1;
		}

		rpos += len;
		wpos += len;
		rest -= len;
	}

	return size;
}
