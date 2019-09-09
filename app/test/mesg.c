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
#include <ipc.h>
#include <services.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#undef FORCE_NEWLINE

#define MYNAME "mesg"

#define MSG_IPC "ipc error"
#define MSG_BUF "buffer error"
#define DELIMITER ": "
#define NEWLINE "\n"

#define OK (0)
#define NG (1)

static void pute(const char *);
static void puterror(const char *);
static int exec(const int);


static void pute(const char *str)
{
	write(STDERR_FILENO, str, strlen(str));
}

static void puterror(const char *message)
{
	pute(MYNAME);
	pute(DELIMITER);
	pute(message);
	pute(NEWLINE);
}

static int exec(const int out)
{
	syslog_t pk;
	pk.Tread.type = Tread;
	pk.Tread.fid = channel_syslog;
	pk.Tread.count = sizeof(pk.Rread.data);

	ssize_t size = ipc_call(PORT_SYSLOG, &pk, sizeof(pk.Tread));
	if (size < 0) {
		puterror(MSG_IPC);
		return NG;
	}

	size = pk.Rread.count;
	if (size < 0) {
		puterror(MSG_BUF);
		return NG;
	}

	if (!size)
		return NG;

	char buf[sizeof(pk.Rread.data) + 2];
	memcpy(buf, pk.Rread.data, size);

#ifdef FORCE_NEWLINE
	if (buf[size - 1] != '\n')
		buf[size++] = '\n';
#endif
	buf[size] = '\0';
	write(out, buf, size);

	return OK;
}

int main(int argc, char **argv)
{
	while (!exec(STDOUT_FILENO));

	return OK;
}
