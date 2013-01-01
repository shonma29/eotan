/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/fd765a/spec.c,v 1.3 1999/12/19 10:59:20 naniwa Exp $ */

#include <string.h>
#include "../../lib/libserv/libserv.h"
#include "fd.h"

struct spec	fd_spec[] =
{
  /* name    head  sector  length    dtl  lencode  gap3  gapskip    srt    hut    hlt     nd  dmachan  retry  motor            */
  /*  {  "2HD",	2,	18,   512,  0xff,       2,   42,     0x2a, 0x0C,  0x0f,  0x08,   0x0,       2,    10,     0 },*/
  {  "2HD",	2,	18,   512,  0xff,       2,   42,     0x1B, 0x0C,  0x0f,  0x08,   0x0,       2,    10,     0 },
  {  "2HC",     2,     15,    512,  0xff,       2,   84,      27,  0x0d,  0x06,  0x0e,   0x0,       2,     3,     0 },	/* 2HC */
  {   NULL,     0,      0,      0,     0,       0,    0,       0,     0,     0,     0,     0,       0,     0,     0 }
};


/*
 * FD の種類名をキーにして、Specification (の入った構造体へのポインタ)を返す。
 */
struct spec *
get_fdspec (B *name)
{
  W	i;

  for (i = 0; fd_spec[i].typename != NULL; i++)
    {
/*      dbg_printf ("compare: <%s>, <%s>\n", name, fd_spec[i].typename); */
      if (strcmp (name, (B *)(fd_spec[i].typename)) == 0)
	{
	  return (&(fd_spec[i]));
	}
    }
  dbg_printf ("fd: get_fdspec error.(name = %s)\n", name);
  return (NULL);	
}

