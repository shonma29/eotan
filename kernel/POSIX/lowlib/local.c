/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/local.c,v 1.2 1999/07/21 15:05:07 naniwa Exp $ */


static char rcsid[] = "$Id: local.c,v 1.2 1999/07/21 15:05:07 naniwa Exp $";


#include "lowlib.h"


/* プロセス固有の情報を管理する */

#ifdef notdef
struct posix_process_info	*posix_info;
#else
struct lowlib_data *posix_info;
#endif
