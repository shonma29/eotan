#ifndef _STDIO_H_
#define _STDIO_H_
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
#include <stddef.h>

#define BUFSIZ (8192)
#define FOPEN_MAX (16)

#define EOF (-1)
#define __CHR_LF '\n'

#define STDIN_FILENO (0)
#define STDOUT_FILENO (1)
#define STDERR_FILENO (2)

#define __FILE_MODE_EOF (0x01)
#define __FILE_MODE_READABLE (0x02)
#define __FILE_MODE_WRITABLE (0x04)
#define __FILE_MODE_APPEND (0x08)
#define __FILE_MODE_BLOCK (0x10)
#define __FILE_MODE_DIRTY (0x20)

typedef struct {
	unsigned int mode;
	size_t pos;
	size_t len;
	int fd;
	size_t buf_size;
	unsigned long long int seek_pos;
	unsigned char buf[BUFSIZ];
} FILE;

extern FILE __libc_files[];

#define stdin (&(__libc_files[STDIN_FILENO]))
#define stdout (&(__libc_files[STDOUT_FILENO]))
#define stderr (&(__libc_files[STDERR_FILENO]))

static inline int isEof(const FILE *stream)
{
	return stream->mode & __FILE_MODE_EOF;
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

static inline int isBlock(const FILE *stream)
{
	return stream->mode & __FILE_MODE_BLOCK;
}

static inline int isDirty(const FILE *stream)
{
	return stream->mode & __FILE_MODE_DIRTY;
}

extern int fflush(FILE *);

extern int fgetc(FILE *);
extern int getchar(void);

extern int fputc(int, FILE *);
extern int putchar(int);
extern int fputs(const char *, FILE *);
extern int puts(const char *);

extern int fprintf(FILE *, const char *, ...);
extern int printf(const char *, ...);

static inline int getc(FILE *stream)
{
	return fgetc(stream);
}

static inline int putc(int c, FILE *stream)
{
	return fputc(c, stream);
}

#endif
