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
#include <nerve/kcall.h>
#include "../../lib/libserv/libserv.h"

#define BUFSIZ 16
#define MIN_AUTO_PORT 49152
#define STATIC_PORT 49151
#define MYNAME "dumb"

static ID port = 0;

static int test_open();
static int test_receive();
static int test_close();


static int test_open(void)
{
	T_CPOR pk_cpor = { TA_TFIFO, BUFSIZ, BUFSIZ };
	ID dupport;

	port = kcall->port_open(0);
	log_notice(MYNAME ": test_open_3 result = %d\n", port);
	if (port != E_PAR)	return 0;

	pk_cpor.poratr = TA_TPRI;
	port = kcall->port_open(&pk_cpor);
	log_notice(MYNAME ": test_open_4 result = %d\n", port);
	if (port != E_RSATR)	return 0;

	pk_cpor.poratr = TA_TFIFO;
	port = kcall->port_open(&pk_cpor);
	log_notice(MYNAME ": test_open_5 result  = %d\n", port);
	if (port != E_OK)	return 0;
	port = STATIC_PORT;

	dupport = kcall->port_open(&pk_cpor);
	log_notice(MYNAME ": test_open_6 result = %d\n", dupport);
	if (dupport != E_OBJ)	return 0;

	return 1;
}

static int test_receive(void)
{
	unsigned char buf[BUFSIZ];
	RDVNO rdvno;
	ER_UINT size;
	INT i;
	ER result;

/*	size = kcall->port_accept(port, 1, &rdvno, buf);
	log_notice(MYNAME ": test_receive_1 size = %d\n", size);
	if (size != E_NOSPT)	return 0;

	size = kcall->port_accept(0, 0xffffffff, &rdvno, buf);
	log_notice(MYNAME ": test_receive_2 size = %d\n", size);
	if (size != E_NOEXS)	return 0;
*/
	for (;;) {
/*
		size = kcall->port_accept(port, &rdvno, 0);
		log_notice(MYNAME ": test_receive_3 rdvno = %d, size = %d\n",
				rdvno, size);
		if (size != E_PAR)	return 0;
*/
		log_notice(MYNAME ": test_receive_4 port = %d\n", port);
		size = kcall->port_accept(port, &rdvno, buf);
		log_notice(MYNAME ": test_receive_4 rdvno = %d, size = %d\n",
				rdvno, size);

		if (size < 0)	break;

		for (i = 0; i < size; i++) {
			log_notice(MYNAME ": buf[%d] = %c\n", i, buf[i]);
		}

		buf[0] = 'd';
		buf[1] = 'u';
		buf[2] = 'm';
		buf[3] = 'b';

		result = kcall->port_reply(0, buf, 4);
		log_notice(MYNAME ": test_reply_1 result = %d\n", result);
		if (result != E_OBJ)	return 0;

		result = kcall->port_reply(rdvno, buf, 17);
		log_notice(MYNAME ": test_reply_2 result = %d\n", result);
		if (result != E_PAR)	return 0;

		result = kcall->port_reply(rdvno, 0, 4);
		log_notice(MYNAME ": test_reply_3 result = %d\n", result);
		if (result != E_PAR)	return 0;

		result = kcall->port_reply(rdvno, buf, 4);
		log_notice(MYNAME ": test_reply_4 result = %d\n", result);
		if (result != E_OK)	return 0;

		break;
	}

	return 1;
}

static int test_close(void)
{
	ER result;

	result = kcall->port_close();
	log_notice(MYNAME ": test_close_2 result = %d\n", result);
	if (result != E_OK)	return 0;

	result = kcall->port_close();
	log_notice(MYNAME ": test_close_3 result = %d\n", result);
	if (result != E_NOEXS)	return 0;

	port = 0;
	return 1;
}

void start(VP_INT exinf)
{
	log_info(MYNAME ": start\n");

	if (test_open())	test_receive();

	if (port > 0)	test_close();

	log_info(MYNAME ": exit\n");

	kcall->thread_end_and_destroy();
}
