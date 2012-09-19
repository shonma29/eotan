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
#include "../../include/itron/types.h"
#include "../../include/itron/struct.h"
#include "../../include/itron/syscall.h"
#include "../../include/itron/rendezvous.h"
#include "../../include/itron/errno.h"

#define BUFSIZ 16

static ID      port;

static int initialize();
static void main();


static int test(void)
{
	T_CPOR pk_cpor = { TA_TFIFO, BUFSIZ, BUFSIZ };

	dbg_printf("[DUMB] test1 acre_por port = %d\n",
			acre_por(0) == E_PAR);

	pk_cpor.poratr = TA_TPRI;
	dbg_printf("[DUMB] test2 acre_por port = %d\n",
			acre_por(&pk_cpor) == E_RSATR);
	pk_cpor.poratr = TA_TFIFO;
}

void start(void)
{
	dbg_printf("[DUMB] start\n");
	test();
	if (initialize())	main();

	if (port > 0)	dbg_printf("[DUMB] del_por port = %d\n", del_por(port));

	dbg_printf("[DUMB] exit\n");
	exd_tsk();
}

static int initialize(void)
{
	T_CPOR pk_cpor = { TA_TFIFO, BUFSIZ, BUFSIZ };

	port = acre_por(&pk_cpor);
	dbg_printf("[DUMB] acre_por port = %d\n", port);
	return (port > 0);
}

static void main(void)
{
	unsigned char buf[BUFSIZ];

	for (;;) {
		RDVNO rdvno;
		INT size = acp_por(port, 0xffffffff, &rdvno, buf);
		INT i;
		ER result;

		dbg_printf("[DUMB] acp_por rdvno = %d, size = %d\n",
				rdvno, size);

		if (size < 0)	break;

		for (i = 0; i < size; i++) {
			dbg_printf("[DUMB] buf[%d] = %c\n", i, buf[i]);
		}

		buf[0] = 'd';
		buf[1] = 'u';
		buf[2] = 'm';
		buf[3] = 'b';

		dbg_printf("[DUMB] rpl_rdb result = %d\n",
				rpl_rdv(rdvno, buf, 4));
	}
}
