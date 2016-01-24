/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2003, Tomohide Naniwa

*/
/* config.h - ファイルシステム設定用のヘッダファイル
 *
 *
 *
 */

#ifndef __FS_CONFIG_H__
#define __FS_CONFIG_H__	1


/* =========================== マクロ定義 =========================== */

/* 各種制限値 */
#define MAX_BODY_SIZE	1024
#define MAX_MOUNT	5
#define MAX_INODE	200
#define MAX_OPEN	100
#define MAX_NAMELEN	255
#define MAX_PROCESS	100

#define PROC_NAME_LEN	35

#define CACHE_BLOCK_SIZE	512

#endif				/* __FS_CONFIG_H__ */
