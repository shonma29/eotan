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
#define MIN_AUTO_PORT 49152
#define STATIC_PORT 49151

static ID      port;

static int initialize();
static int test_acp_por();


static int test_cre_por(void)
{
	T_CPOR pk_cpor = { TA_TFIFO, BUFSIZ, BUFSIZ };
	ID dupport;

	port = cre_por(0, &pk_cpor);
	dbg_printf("[DUMB] test_cre_por_1 result = %d\n", port);
	if (port != E_ID)	return 0;

	port = cre_por(MIN_AUTO_PORT, &pk_cpor);
	dbg_printf("[DUMB] test_cre_por_2 result = %d\n", port);
	if (port != E_ID)	return 0;

	port = cre_por(STATIC_PORT, 0);
	dbg_printf("[DUMB] test_cre_por_3 result = %d\n", port);
	if (port != E_PAR)	return 0;

	pk_cpor.poratr = TA_TPRI;
	port = cre_por(STATIC_PORT, &pk_cpor);
	dbg_printf("[DUMB] test_cre_por_4 result = %d\n", port);
	if (port != E_RSATR)	return 0;

	pk_cpor.poratr = TA_TFIFO;
	port = cre_por(STATIC_PORT, &pk_cpor);
	dbg_printf("[DUMB] test_cre_por_5 result  = %d\n", port);
	if (port != E_OK)	return 0;
	port = STATIC_PORT;

	dupport = cre_por(STATIC_PORT, &pk_cpor);
	dbg_printf("[DUMB] test_cre_por_6 result = %d\n", dupport);
	if (dupport != E_OBJ)	return 0;

	return 1;
}

static int test_acre_por(void)
{
	T_CPOR pk_cpor = { TA_TFIFO, BUFSIZ, BUFSIZ };

	port = acre_por(0);
	dbg_printf("[DUMB] test_acre_por_1 port = %d\n", port);
	if (port != E_PAR)	return 0;

	pk_cpor.poratr = TA_TPRI;
	port = acre_por(&pk_cpor);
	dbg_printf("[DUMB] test_acre_por_2 port = %d\n", port);
	if (port != E_RSATR)	return 0;

	pk_cpor.poratr = TA_TFIFO;
	port = acre_por(&pk_cpor);
	dbg_printf("[DUMB] test_acre_por_3 port = %d\n", port);
	if (port <= 0)	return 0;

	return 1;
}

void start(void)
{
	dbg_printf("[DUMB] start\n");

	if (test_acre_por())	test_acp_por();

	if (port > 0)	dbg_printf("[DUMB] del_por port = %d\n", del_por(port));

	dbg_printf("[DUMB] exit\n");

	exd_tsk();
}

static int test_acp_por(void)
{
	unsigned char buf[BUFSIZ];
	RDVNO rdvno;
	ER_UINT size;
	INT i;
	ER result;

	size = acp_por(port, 1, &rdvno, buf);
	dbg_printf("[DUMB] test_acp_por_1 size = %d\n", size);
	if (size != E_NOSPT)	return 0;

	size = acp_por(0, 0xffffffff, &rdvno, buf);
	dbg_printf("[DUMB] test_acp_por_2 size = %d\n", size);
	if (size != E_NOEXS)	return 0;

	for (;;) {
		dbg_printf("[DUMB] test_acp_por_3 port = %d\n", port);
		size = acp_por(port, 0xffffffff, &rdvno, buf);
		dbg_printf("[DUMB] test_acp_por_3 rdvno = %d, size = %d\n",
				rdvno, size);

		if (size < 0)	break;

		for (i = 0; i < size; i++) {
			dbg_printf("[DUMB] buf[%d] = %c\n", i, buf[i]);
		}

		buf[0] = 'd';
		buf[1] = 'u';
		buf[2] = 'm';
		buf[3] = 'b';

		result = rpl_rdv(0, buf, 4);
		dbg_printf("[DUMB] test_rpl_rdv_1 result = %d\n", result);
		if (result != E_OBJ)	return 0;

		result = rpl_rdv(rdvno, buf, 17);
		dbg_printf("[DUMB] test_rpl_rdv_2 result = %d\n", result);
		if (result != E_PAR)	return 0;

		result = rpl_rdv(rdvno, buf, 4);
		dbg_printf("[DUMB] test_rpl_rdv_3 result = %d\n", result);
		if (result != E_OK)	return 0;
	}

	return 1;
}
