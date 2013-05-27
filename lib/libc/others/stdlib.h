/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002-2003, Tomohide Naniwa

*/

#ifndef __STDLIB_H__
#define __STDLIB_H__	1

#include <core/types.h>

#define BUFSIZE		1024
#define NFILE		10

#define STDIN	0
#define STDOUT	1
#define STDERR	2


struct _file
{
  ID	device;
  W	count;
  W	length;
  W	bufsize;
  UB	buf[BUFSIZE];
};

typedef struct _file	FILE;

extern FILE	__file_table__[];

#define stdin	(&__file_table__[0])
#define stdout	(&__file_table__[1])
#define stderr	(&__file_table__[2])


extern W	getc (FILE *port);
extern W        fflush(FILE *port);
extern W	putc (W ch, FILE *port);
extern W	putchar (W ch);
extern W	fputs (B *line, FILE *port);
extern W	puts (B *line);
extern W	printf (B *fmt,...);
extern W	fprintf (FILE *port, B *fmt,...);
extern W isprint (W ch);
extern W isspace (W ch);
extern W isdigit (W ch);

#endif /* __STDLIB_H__ */
