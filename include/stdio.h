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

extern W	putchar (W ch);
extern W	fputs (B *line, FILE *port);
extern W	puts (B *line);
extern W	printf (B *fmt,...);
extern W	fprintf (FILE *port, B *fmt,...);

#endif /* __STDIO_H__ */
