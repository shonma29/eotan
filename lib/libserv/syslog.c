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
#include <stdarg.h>
#include <string.h>
#include <syslog.h>
#include <dev/device.h>
#include <nerve/kcall.h>

static int send(const syslog_t *);
static int _putc(const char, syslog_t *);


void syslog(const int priority, const char *message, ...)
{
	syslog_t packet;
	packet.Twrite.type = Twrite;
	packet.Twrite.priority = priority;
	packet.Twrite.count = 0;

	va_list ap;
	va_start(ap, message);
	vnprintf2((int (*)(char, void *)) _putc, &packet, message, ap);

	if (packet.Twrite.count)
		send(&packet);
}

static int send(const syslog_t *packet)
{
	return kcall->ipc_call(PORT_SYSLOG, (void *) packet,
			sizeof(packet->Twrite)
					- sizeof(packet->Twrite.data)
					+ packet->Twrite.count);
}

static int _putc(const char ch, syslog_t *packet)
{
	packet->Twrite.data[packet->Twrite.count] = ch;
	packet->Twrite.count++;

	if (packet->Twrite.count == SYSLOG_MAX_LENGTH - 1) {
		packet->Twrite.data[SYSLOG_MAX_LENGTH - 1] = '\n';
		packet->Twrite.count = SYSLOG_MAX_LENGTH;
		send(packet);
		//TODO why 2nd block can't put?
		packet->Twrite.count = 0;
	}

	return 0;
}
