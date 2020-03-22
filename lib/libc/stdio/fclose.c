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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "macros.h"


int fclose(FILE *stream)
{
	if (!isOpen(stream)) {
		_set_local_errno(EBADF);
		return EOF;
	}

	int result;
	do {
		if (fflush(stream)) {
			int error_no = errno;

			close(stream->fd);
			_set_local_errno(error_no);
			result = EOF;
			break;
		}

		if (close(stream->fd)) {
			result = EOF;
			break;
		}

		result = 0;
	} while (false);

	stream->mode = 0;
	stream->pos = 0;
	stream->len = 0;
	stream->fd = -1;
	stream->buf_size = 0;
	stream->seek_pos = 0;

	if (stream->buf) {
		free(stream->buf);
		stream->buf = NULL;
	}

	return result;
}
