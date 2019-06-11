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
#include <errno.h>
#include <stdio.h>
#include "macros.h"


int fputc(int c, FILE *stream)
{
	if (ferror(stream))
		return EOF;

	if (!isWritable(stream)) {
		_set_local_errno(EBADF);
		return EOF;
	}

	if (stream->pos >= stream->buf_size)
		if (__sweep_buffer(stream))
			return EOF;

	stream->mode |= __FILE_MODE_DIRTY;
	stream->buf[stream->pos++] = c & 0xff;

	if (stream->pos > stream->len)
		stream->len = stream->pos;

	if (!isBlock(stream))
		if (__sweep_buffer(stream))
			return EOF;

	return c;
}
