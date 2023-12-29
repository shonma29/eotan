#ifndef _LIBC_STDIO_MACROS_H_
#define _LIBC_STDIO_MACROS_H_
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
#include <stdbool.h>
#include <stdio.h>

typedef struct {
	size_t len;
	size_t max;
	char *buf;
} CharBuffer;

static inline int isOpen(const FILE *stream)
{
	return stream->mode & (__FILE_MODE_READABLE | __FILE_MODE_WRITABLE);
}

static inline int isReadable(const FILE *stream)
{
	return stream->mode & __FILE_MODE_READABLE;
}

static inline int isWritable(const FILE *stream)
{
	return stream->mode & __FILE_MODE_WRITABLE;
}

static inline int isAppend(const FILE *stream)
{
	return stream->mode & __FILE_MODE_APPEND;
}

static inline bool isLineBuffered(const FILE *stream)
{
	return ((stream->mode & __MASK_BUFFERED) == _IOLBF);
}

static inline int isDirty(const FILE *stream)
{
	return stream->mode & __FILE_MODE_DIRTY;
}

extern int __putc(const char, CharBuffer *);
extern int __fill_buffer(void *, const size_t, FILE *);
extern int __sweep_buffer(FILE *);

extern int __parse_file_mode(const char *, int *, int *);

extern int vnprintf2(int (*)(const char, void*), void *,
		const char *, va_list *);

#endif
