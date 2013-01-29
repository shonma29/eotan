/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native.h,v 1.2 2000/06/01 08:43:10 naniwa Exp $ */


#ifndef __NATIVE_H__

#include "../../servers/fs/fs.h"

#define NAME_MAX	14


#ifdef notdef
struct dirent
{
  long			d_ino;
  unsigned long		d_off;
  unsigned short	d_reclen;
  char			d_name[NAME_MAX + 1];
};
#endif

struct sigaction
{
};

typedef int	sigset_t;

struct tms
{
};

typedef unsigned long	clock_t;


extern struct lowlib_data *lowlib_data;

extern int call_lowlib(const int svcno, ...);
extern ER _make_connection(W wOperation, struct posix_request *req);

#endif
