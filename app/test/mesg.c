#/*
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
#include <string.h>
#include <unistd.h>
#include <itron/types.h>
#include <itron/rendezvous.h>
#include "../../lib/libc/others/stdlib.h"
#include "../../servers/kernlog/kernlog.h"

#undef FORCE_NEWLINE

#define MYNAME "mesg"

#define MSG_PORT "port error"
#define MSG_BUF "buffer error"
#define DELIMITER ": "
#define NEWLINE "\n"

#define OK (0)
#define NG (1)

static void pute(char *str);
static void puterror(char *message);
static int exec(int out);

static void pute(char *str) {
	write(STDERR, str, strlen(str));
}

static void puterror(char *message) {
	pute(MYNAME);
	pute(DELIMITER);
	pute(message);
	pute(NEWLINE);
}

static int exec(int out) {
	devmsg_t pk;
	ER_UINT size;
	UB buf[DEV_BUF_SIZE + 2];

	pk.req.header.msgtyp = DEV_REA;
/*
	pk.req.header.tskid = 0;
*/
	pk.req.body.rea_req.dd = DESC_SYSLOG;
	pk.req.body.rea_req.start = 0;
	pk.req.body.rea_req.size = DEV_BUF_SIZE;

	size = cal_por(PORT_SYSLOG, 0xffffffff, &pk, sizeof(pk.req));

	if (size < 0) {
		puterror(MSG_PORT);
		return NG;
	}

	if (pk.res.body.rea_res.errcd < 0) {
		puterror(MSG_BUF);
		return NG;
	}

	size = pk.res.body.rea_res.a_size;
	if (!size)
		return NG;

	memcpy(buf, pk.res.body.rea_res.dt, size);

#ifdef FORCE_NEWLINE
	if (buf[size - 1] != '\n')
		buf[size++] = '\n';
#endif
	buf[size] = '\0';
	write(out, (char*)buf, size);

	return OK;
}

int main(int argc, char **argv) {
	while (!exec(STDOUT));

	return OK;
}