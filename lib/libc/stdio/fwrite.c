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
#include <string.h>
#include <unistd.h>
#include "macros.h"


size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	if (ferror(stream))
		return 0;

	if (!isWritable(stream)) {
		_set_local_errno(EBADF);
		return 0;
	}

	size_t bytes = size * nmemb;
	size_t rest = bytes;
	int len = stream->buf_size - stream->pos;
	if (len) {
		if (len > rest)
			len = rest;

		memcpy(&(stream->buf[stream->pos]), ptr, len);
		stream->mode |= __FILE_MODE_DIRTY;
		stream->pos += len;
		if (stream->pos > stream->len)
			stream->len = stream->pos;

		rest -= len;
		if (!rest)
			return nmemb;

		ptr = (void*)(((unsigned int)ptr) + len);
	}

	if (!__sweep_buffer(stream)) {
		if (rest < stream->buf_size) {
			if (len > rest)
				len = rest;
	
			memcpy(stream->buf, ptr, len);
			stream->mode |= __FILE_MODE_DIRTY;
			stream->pos += len;
			stream->len = stream->pos;
			rest -= len;
		} else {
			len = write(stream->fd, ptr, rest);
			if (len < 0)
				stream->mode |= __FILE_MODE_ERROR;
			else if (len > 0) {
				stream->seek_pos += len;
				rest -= len;
			}
		}
	}

	return ((bytes - rest) / size);
}
