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
#include <stdarg.h>
#include <string.h>
#include "../libserv/libserv.h"

#define MAX_BUF (80)

typedef struct {
	size_t len;
	char *buf;
} CharBuffer;

static int _putc(const char, CharBuffer *);


int dbg_printf(const char *format, ...)
{
	char str[MAX_BUF];
	CharBuffer buf = { 0, str };
	int len;
	va_list ap;

	va_start(ap, format);
	len = vnprintf2((int (*)(char, void*))_putc, &buf,
			(char*)format, ap);
	str[buf.len] = '\0';

	syslog(str);
	return len;
}

static int _putc(const char ch, CharBuffer *buf)
{
	if (buf->len < MAX_BUF - 1)
		buf->buf[buf->len++] = ch;

	return 0;
}