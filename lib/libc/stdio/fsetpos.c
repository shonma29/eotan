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
#include <unistd.h>
#include "macros.h"


int fsetpos(FILE *stream, const fpos_t *pos)
{
	//TODO set errno
	if (ferror(stream))
		return -1;

	fpos_t next = *pos;
	fpos_t end = stream->seek_pos + stream->len;
	off_t param = *pos;

	if ((next < stream->seek_pos)
			|| (next >= end)) {
		int result = fflush(stream);
		if (result)
			return result;

		param = lseek(stream->fd, param, SEEK_SET);
		if (param == (off_t)(-1))
			return -1;

		stream->seek_pos = param;
		stream->len = stream->pos = 0;
	} else
		stream->pos = next - stream->seek_pos;

	stream->mode &= ~__FILE_MODE_EOF;
	return 0;
}
