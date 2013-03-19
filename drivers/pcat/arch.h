/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* pc98.h
 *
 * Discription
 *	IBMPC に依存する部分の定義
 */

#ifndef __PCAT_ARCH_H__
#define __PCAT_ARCH_H__	1

#define MASTER_8259A_COM	0x20
#define MASTER_8259A_DATA	0x21

#define SLAVE_8259A_COM		0xa0
#define SLAVE_8259A_DATA	0xa1

#define INT_TIMER		32
#define INT_KEYBOARD		33

#define RESERVED_MEMORY		(1024 * 1024)

#endif /* __PCAT_ARCH_H__ */
