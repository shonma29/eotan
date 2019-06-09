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
#include <string.h>
#include "macros.h"


size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	//TODO set errno
	if (ferror(stream) || feof(stream))
		return 0;

	if (!isReadable(stream))
		return 0;

	size_t bytes = size * nmemb;
	size_t rest = bytes;
	size_t len = stream->len - stream->pos;
	if (len) {
		if (len > rest)
			len = rest;

		memcpy(ptr, &(stream->buf[stream->pos]), len);
		stream->pos += len;
		rest -= len;
		if (!rest)
			return nmemb;

		ptr = (void*)(((unsigned int)ptr) + len);
	}

	if (rest < stream->buf_size) {
		if (!__fill_buffer(stream->buf, stream->buf_size, stream)) {
			len = stream->len;
			if (len > rest)
				len = rest;
	
			memcpy(ptr, stream->buf, len);
			stream->pos += len;
			rest -= len;
		}
	} else if (!__fill_buffer(ptr, rest, stream)) {
		len = stream->len;
		stream->len = 0;
		stream->seek_pos += len;
		rest -= len;
	}

	return ((bytes - rest) / size);
}
