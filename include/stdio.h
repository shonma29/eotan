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
#include <stdarg.h>
#include <stddef.h>

typedef unsigned long long int fpos_t;

#define BUFSIZ (8192)
#define FOPEN_MAX (16)

#define EOF (-1)
#define __CHR_LF '\n'

#define STDIN_FILENO (0)
#define STDOUT_FILENO (1)
#define STDERR_FILENO (2)

#define __FILE_MODE_EOF (0x01)
#define __FILE_MODE_ERROR (0x02)
#define __FILE_MODE_READABLE (0x04)
#define __FILE_MODE_WRITABLE (0x08)
#define __FILE_MODE_APPEND (0x10)
#define __FILE_MODE_UNBUFFERED (0x20)
#define __FILE_MODE_LINE_BUFFERED (0x40)
#define __FILE_MODE_DIRTY (0x80)

#define _IONBF (__FILE_MODE_UNBUFFERED)
#define _IOFBF (0)
#define _IOLBF (__FILE_MODE_LINE_BUFFERED)
#define __MASK_BUFFERED (__FILE_MODE_UNBUFFERED | __FILE_MODE_LINE_BUFFERED)

typedef struct {
	unsigned int mode;
	size_t pos;//TODO why size_t?
	size_t len;
	int fd;
	size_t buf_size;//TODO what if no buffering
	fpos_t seek_pos;
	unsigned char *buf;//TODO what if no buffering
int cnt;
} FILE;

extern FILE __libc_files[];

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

static inline int feof(FILE *stream)
{
	return stream->mode & __FILE_MODE_EOF;
}

static inline int ferror(FILE *stream)
{
	return stream->mode & __FILE_MODE_ERROR;
}

static inline void clearerr(FILE *stream)
{
	stream->mode &= ~(__FILE_MODE_EOF | __FILE_MODE_ERROR);
}

extern int fileno(FILE*);

extern FILE *fopen(const char *, const char *);
extern FILE *fdopen(int fd, const char *);
extern int fclose(FILE *);
extern int fflush(FILE *);

extern int fgetc(FILE *);
extern int getchar(void);
extern char *fgets(char *, int, FILE *);

extern int fputc(int, FILE *);
extern int putchar(int);
extern int fputs(const char *, FILE *);
extern int puts(const char *);

extern int fgetpos(FILE *, fpos_t *);
extern long ftell(FILE *);
extern int fsetpos(FILE *, const fpos_t *);
extern int fseek(FILE *, long, int);
extern void rewind(FILE *);

extern size_t fread(void *, size_t, size_t, FILE *);
extern size_t fwrite(const void *, size_t, size_t, FILE *);

extern int printf(const char *, ...);
extern int fprintf(FILE *, const char *, ...);
extern int vfprintf(FILE *, const char *, va_list);
extern int vprintf(const char *, va_list);
extern int sprintf(char *, const char *, ...);
extern int snprintf(char *, size_t, const char *, ...);
extern int vsprintf(char *, const char *, va_list);
extern int vsnprintf(char *, size_t, const char *, va_list);

static inline int getc(FILE *stream)
{
	return fgetc(stream);
}

static inline int putc(int c, FILE *stream)
{
	return fputc(c, stream);
}

extern void perror(const char *);

#endif
