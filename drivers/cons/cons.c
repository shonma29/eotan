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
#include "cons.h"


int detach(void)
{
	return 0;
}

int open(void)
{
	return 0;
}

int close(const int channel)
{
	return (channel == 0)? 0:(-1);
}

int read(char *outbuf, const int channel,
		const off_t start, const size_t size)
{
	off_t rpos = start;
	off_t wpos = 0;
	size_t rest = size;

	while (rest > 0) {
		ER_UINT result;
		size_t len = (rest < DEV_BUF_SIZE)? rest:DEV_BUF_SIZE;
		devmsg_t packet;

		packet.Tread.operation = operation_read;
		packet.Tread.fid = channel;
		packet.Tread.offset = rpos;
		packet.Tread.count = len;
		packet.Tread.data = &(outbuf[wpos]);

		result = kcall->port_call(PORT_CONSOLE, &packet,
				sizeof(packet.Tread));
		if (result != sizeof(packet.Rread)) {
			log_err("cons: call failed(%d)\n", result);
			return -1;
		}

		else if (packet.Rread.count != len) {
			log_err("cons: call icompletely\n");
			return -1;
		}

		rpos += len;
		wpos += len;
		rest -= len;
	}

	return size;
}

int write(char *inbuf, const int channel,
		const off_t start, const size_t size)
{
	off_t rpos = 0;
	off_t wpos = start;
	size_t rest = size;

	while (rest > 0) {
		ER_UINT result;
		size_t len = (rest < DEV_BUF_SIZE)? rest:DEV_BUF_SIZE;
		devmsg_t packet;

		packet.Twrite.operation = operation_write;
		packet.Twrite.fid = channel;
		packet.Twrite.offset = wpos;
		packet.Twrite.count = len;
		packet.Twrite.data = &(inbuf[rpos]);

		result = kcall->port_call(PORT_CONSOLE, &packet,
				sizeof(packet.Twrite));
		if (result != sizeof(packet.Rwrite)) {
			log_err("cons: call failed(%d)\n", result);
			return -1;
		}

		else if (packet.Rwrite.count != len) {
			log_err("cons: wrote icompletely\n");
			return -1;
		}

		rpos += len;
		wpos += len;
		rest -= len;
	}

	return size;
}
