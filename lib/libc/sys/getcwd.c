/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_getcwd.c,v 1.1 1997/08/31 13:25:20 night Exp $  */

#include <errno.h>
#include <itron/types.h>
#include <lowlib.h>
#include "posix.h"
#include "../libserv/libserv.h"


/* getcwd 
 *
 */
char *
getcwd (char *buf, int size)
{
    struct lowlib_data	*lowlib_data = (struct lowlib_data *)LOWLIB_DATA;

    if (lowlib_data->dpath_len > size) {
	errno = EP_RANGE;
	return (NULL);
    }
    strncpy2(buf, lowlib_data->dpath, size);
    errno = EP_OK;
    return (buf);
}


