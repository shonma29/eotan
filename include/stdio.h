/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002-2003, Tomohide Naniwa

*/

#ifndef __STDIO_H__
#define __STDIO_H__	1

#include <core/types.h>
#include <nstdio.h>

#define NFILE		10

extern FILE	__file_table__[];

#define stdin	(&__file_table__[STDIN_FILENO])
#define stdout	(&__file_table__[STDOUT_FILENO])
#define stderr	(&__file_table__[STDERR_FILENO])


extern W	fgetc (FILE *port);
extern W        fflush(FILE *port);
extern W	fputc (W ch, FILE *port);
extern W	putchar (W ch);
extern W	fputs (B *line, FILE *port);
extern W	puts (B *line);
extern W	printf (B *fmt,...);
extern W	fprintf (FILE *port, B *fmt,...);

#endif /* __STDIO_H__ */
