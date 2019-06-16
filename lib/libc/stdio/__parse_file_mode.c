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
#include <fcntl.h>
#include "macros.h"


int __parse_file_mode(const char *mode, int *file_mode, int *open_mode)
{
	const char *p = mode;
	//TODO support append
	switch (*p) {
	case 'r':
		*open_mode = O_RDONLY;
		*file_mode = __FILE_MODE_READABLE;
		break;
	case 'w':
		*open_mode = O_WRONLY | O_CREAT | O_TRUNC;
		*file_mode = __FILE_MODE_WRITABLE;
		break;
	default:
		return EINVAL;
	}

	p++;
	if (*p == 'b') {
		p++;

		if (*p == 'b')
			return EINVAL;
	}

	if (*p) {
		if (*p != '+')
			return EINVAL;

		p++;
		switch (*p) {
		case '\0':
			break;
		case 'b':
			p++;
			if (*p)
				return EINVAL;
			break;
		default:
			return EINVAL;
		}

		*file_mode = __FILE_MODE_READABLE | __FILE_MODE_WRITABLE;

		switch (*mode) {
		case 'r':
			*open_mode = O_RDWR;
			break;
		case 'w':
			*open_mode = O_RDWR | O_CREAT | O_TRUNC;
			break;
		default:
			break;
		}
	}

	return 0;
}
