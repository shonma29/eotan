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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ipc.h>
#include <services.h>
#include <fs/protocol.h>

static int putline(char *);


static int putline(char *buf)
{
	fsmsg_t message;
	message.header.ident = IDENT;
	message.header.type = Twrite;
	message.Twrite.fid = 7;
	message.Twrite.offset = 0;
	message.Twrite.count = strlen(buf);
	message.Twrite.data = buf;

	int err = ipc_call(PORT_WINDOW, &message, MESSAGE_SIZE(Twrite));
	if (err < 0)
		fprintf(stderr, "call error %d\n", err);

	return err;
}

int main(int argc, char **argv)
{
	
	do {
		time_t t = time(NULL);
		if (t == -1)
			break;//TODO show error

		struct tm tm;
		if (!gmtime_r(&t, &tm))
			break;//TODO show error

		char buf[256];//TODO ugly
		size_t len = strftime(buf, sizeof(buf),
				"\x1b[1K\x1b[H%a %b %d %H:%M", &tm);
		if (!len)
			break;//TODO show error

		putline(buf);

		if (tm.tm_sec < 60)
			sleep(60 - tm.tm_sec);
	} while (true);
	_exit(EXIT_FAILURE);
}
