/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/*
 * $Log: chdir.c,v $
 * Revision 1.3  2000/05/20 12:39:21  naniwa
 * to check as a directry
 *
 * Revision 1.2  2000/05/20 11:57:17  naniwa
 * minor fix
 *
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include <core.h>
#include "fs.h"

W session_get_opened_file(const ID pid, const W fno, struct file **fp)
{
    W err = proc_get_file(pid, fno, fp);

    if (err)
	return err;

    if ((*fp)->f_inode == NULL)
	return EBADF;

    return EOK;
}
