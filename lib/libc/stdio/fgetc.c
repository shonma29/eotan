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


int fgetc(FILE *stream)
{
	//TODO set errno
	if (ferror(stream) || feof(stream))
		return EOF;

	if (!isReadable(stream))
		return EOF;

	if (stream->pos >= stream->len) {
		int len;

		stream->pos = 0;
		stream->seek_pos += stream->len;
		len = read(stream->fd, stream->buf, stream->buf_size);

		if (len == 0) {
			stream->len = 0;
			stream->mode |= __FILE_MODE_EOF;
			return EOF;

		} else if (len < 0) {
			stream->len = 0;
			stream->mode |= __FILE_MODE_ERROR;
			return EOF;

		} else
			stream->len = len;
	}

	return (int)(stream->buf[stream->pos++]);
}