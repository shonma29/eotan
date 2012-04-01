#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__
/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/

/* $Id: filesystem.h,v 1.1 1998/11/26 11:55:24 monaka Exp $ */
static char rcsid[] = "$Id: filesystem.h,v 1.1 1998/11/26 11:55:24 monaka Exp $";

/*
 * $Log: filesystem.h,v $
 * Revision 1.1  1998/11/26 11:55:24  monaka
 *
 * First version.
 *
 */

extern ER init_filesystem (ID root_device, UW part, B *type);
extern ER get_root (struct filesystem *fs, struct filedata **fp);
extern ER lookup (struct filedata *parent, B *fname, struct filedata **ret_fp);
extern ER get_attribute (struct filedata *fp, struct fstat *ret_st);
extern ER get_directory_list (struct filedata *dir, struct directory *ret);
extern ER read (struct filedata *fp, W offset, W size, B *buf);
extern ER write (struct filedata *fp, W offset, W size, B *buf);

#endif
