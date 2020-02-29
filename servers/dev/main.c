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
#include <services.h>
#include <nerve/kcall.h>
#include "../../lib/libserv/libserv.h"
#include "dev.h"
#include "table.h"

static void doit(void);
static ER initialize(void);


static void doit(void)
{
	for (;;) {
		devmsg_t packet;
		int tag;
		int size = kcall->ipc_receive(PORT_DEV, &tag, &packet);
		if (size < 0) {
			log_err(MYNAME ": receive failed %d\n", size);
			break;
		}

		if (size == sizeof(packet)) {
			if (packet.type == devmsg_find) {
				device_info_t *d = dev_find(
						(char *) packet.arg1);
				if (d && d->driver) {
					packet.arg1 = E_OK;
					packet.arg2 = (int) d;
					packet.arg3 = 0;
				} else {
					packet.arg1 = E_NOEXS;
					packet.arg2 = 0;
					packet.arg3 = 0;
				}
			} else {
				packet.arg1 = E_NOSPT;
				packet.arg2 = 0;
				packet.arg3 = 0;
			}
		} else {
			packet.arg1 = E_PAR;
			packet.arg2 = 0;
			packet.arg3 = 0;
		}

		ER result = kcall->ipc_send(tag, &packet, sizeof(packet));
		if (result)
			log_err(MYNAME ": reply(0x%x) failed %d\n",
					tag, result);
	}
}

static ER initialize(void)
{
	if (!dev_initialize())
		return E_NOMEM;

	T_CPOR pk_cpor = { TA_TFIFO, sizeof(devmsg_t), sizeof(devmsg_t) };
	ER result = kcall->ipc_open(&pk_cpor);
	if (result) {
		log_err(MYNAME ": open failed %d\n", result);
		return result;
	}
	return result;
}

void start(VP_INT exinf)
{
	ER error = initialize();
	if (error)
		log_err(MYNAME ": open failed %d\n", error);
	else {
		log_info(MYNAME ": start\n");
		doit();
		log_info(MYNAME ": end\n");

		error = kcall->ipc_close();
		if (error)
			log_err(MYNAME ": close failed %d\n", error);
	}

	kcall->thread_end_and_destroy();
}
