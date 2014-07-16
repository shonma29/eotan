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


int fputc(int c, FILE *stream)
{
	//TODO set errno
	if (ferror(stream))
		return EOF;

	if (!isWritable(stream))
		return EOF;

	stream->mode |= __FILE_MODE_DIRTY;
	stream->buf[stream->pos++] = c & 0xff;

	if (isReadable(stream)) {
		if (stream->pos >= stream->len) {
			if (fflush(stream))
				return EOF;

			fgetc(stream);
			stream->pos = 0;
			return c;
		}

	} else if (stream->pos > stream->len) {
		stream->len = stream->pos;

		if (stream->len >= stream->buf_size) {
			if (fflush(stream))
				return EOF;

			stream->len = stream->pos = 0;
			return c;
		}
	}

	if (c == __CHR_LF)
		if (!isBlock(stream)) {
			if (fflush(stream))
				return EOF;

			if (!isReadable(stream))
				stream->len = stream->pos = 0;
		}

	return c;
}
