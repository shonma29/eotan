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
#include <stdlib.h>
#include <fcntl.h>
#include "macros.h"

static FILE *_fopen(FILE *, const char *, const char *);


FILE *fopen(const char *path, const char *mode)
{
	for (int i = 0; i < FOPEN_MAX; i++)
		if (!isOpen(&(__libc_files[i])))
			return _fopen(&(__libc_files[i]), path, mode);

	_set_local_errno(EMFILE);
	return NULL;
}

static FILE *_fopen(FILE *stream, const char *path, const char *mode)
{
	unsigned char *buf = malloc(BUFSIZ);
	if (!buf) {
		_set_local_errno(ENOMEM);
		return NULL;
	}

	int open_mode;
	int file_mode;
	int result = __parse_file_mode(mode, &file_mode, &open_mode);
	if (result) {
		_set_local_errno(result);
		return NULL;
	}

	int fd = open(path, open_mode);
	if (fd < 0)
		return NULL;

	stream->mode = file_mode;
	stream->pos = 0;
	stream->len = 0;
	stream->fd = fd;
	stream->buf_size = BUFSIZ;
	stream->seek_pos = 0;
	stream->buf = buf;
	return stream;
}
