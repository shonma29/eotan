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
#include <console.h>
#include <device.h>
#include <services.h>
#include <stddef.h>
#include <string.h>
#include <mpu/memory.h>
#include <nerve/config.h>
#include <nerve/kcall.h>
#include <libserv.h>
#include "hmi.h"
#include "keyboard.h"
#include "mouse.h"

#ifdef USE_VESA
#include <boot/vesa.h>

#define MAX_WINDOW (16)

typedef struct {
	UW width;
	UW height;
} Screen;

typedef struct {
	UW left;
	UW top;
	UW width;
	UW height;
	UW cursor_x;
	UW cursor_y;
	UW color;
	UW enabled;
} window_t;

//static window_t window[MAX_WINDOW];

extern Console *getConsole(void);
extern void put(const unsigned int start, const size_t size,
		const unsigned char *buf);
#else
#include <cga.h>
#endif

static Console *cns;

static ER check_param(const UW start, const UW size);
static ER_UINT write(const UW dd, const UW start, const UW size,
		const UB *inbuf);
static UW execute(devmsg_t *message);
static ER accept(void);
static ER initialize(void);
static ER_UINT dummy_read(const UW start, const UW size, UB *outbuf);

static ER_UINT (*reader)(const UW start, const UW size, UB *outbuf) =
		dummy_read;


static ER check_param(const UW start, const UW size)
{
/*
	if (start)	return E_PAR;
*/
	if (size > DEV_BUF_SIZE)	return E_PAR;

	return E_OK;
}

static ER_UINT write(const UW dd, const UW start, const UW size,
		const UB *inbuf)
{
	ER_UINT result = check_param(start, size);
	size_t i;

	if (result)	return result;

	if (dd)
#ifdef USE_VESA
		put(start, size, inbuf);
#else
		;
#endif
	else {
		for (i = 0; i < size; i++)
			cns->putc(inbuf[i]);
	}

	return size;
}

static UW execute(devmsg_t *message)
{
	ER_UINT result;
	UW size = 0;

	switch (message->Rread.operation) {
	case operation_read:
		result = reader(message->Rread.offset,
				message->Rread.length, message->Rread.data);
		message->Tread.length = result;
		size = sizeof(message->Tread);
		break;

	case operation_write:
		result = write(message->Rwrite.channel,
				message->Rwrite.offset,
				message->Rwrite.length,
				message->Rwrite.data);
		message->Twrite.length = result;
		size = sizeof(message->Twrite);
		break;

	default:
		break;
	}

	return size;
}

static ER accept(void)
{
	devmsg_t *message;
	RDVNO rdvno;
	ER_UINT size;
	ER result;

	size = kcall->port_accept(PORT_CONSOLE, &rdvno, &message);
	if (size < 0) {
		dbg_printf("hmi: acp_por error=%d\n", size);
		return size;
	}

	execute(message);
	result = kcall->port_reply(rdvno, &message, 0);
	if (result) {
		dbg_printf("hmi: rpl_rdv error=%d\n", result);
	}

	return result;
}

static ER initialize(void)
{
	W result;
	T_CPOR pk_cpor = {
			TA_TFIFO,
			sizeof(devmsg_t),
			sizeof(devmsg_t)
	};

#ifdef USE_VESA
	cns = getConsole();
#else
	cns = getConsole((const UH*)kern_p2v((void*)CGA_VRAM_ADDR));
#endif
	cns->cls();
	cns->locate(0, 0);

	result = kcall->port_create(PORT_CONSOLE, &pk_cpor);
	if (result) {
		dbg_printf("hmi: acre_por error=%d\n", result);

		return result;
	}

	return E_OK;
}

static ER_UINT dummy_read(const UW start, const UW size, UB *outbuf)
{
	return E_NOSPT;
}

void start(VP_INT exinf)
{
	T_CTSK pk = {
		TA_HLNG,
		(VP_INT)NULL,
		(FP)mouse_accept,
		pri_server_middle,
		KTHREAD_STACK_SIZE,
		NULL,
		NULL,
		NULL
	};

	if (initialize() == E_OK) {
		dbg_printf("hmi: start\n");

		if (keyboard_initialize() == E_OK)
			reader = keyboard_read;

		if (mouse_initialize() == E_OK) {
			ER_ID t2 = kcall->thread_create_auto(&pk);

			dbg_printf("hmi: mouse=%d\n", t2);

			if (t2 > 0)
				kcall->thread_start(t2);
		}

		while (accept() == E_OK);

		kcall->port_destroy(PORT_CONSOLE);
		dbg_printf("hmi: end\n");
	}

	kcall->thread_end_and_destroy();
}
